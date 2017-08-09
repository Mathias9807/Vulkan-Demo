/*
 * Simple Vulkan application
 * 
 * Copyright (c) 2016 by Mathias Johansson
 * 
 * This code is licensed under the MIT license 
 * 		https://opensource.org/licenses/MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "util/vulkan.h"

void createGraphicsPipeline();
void setupFramebuffers();
void beginRenderPass(VkCommandBuffer comBuffer, VkFramebuffer framebuffer);

int width = 512, height = 512;
SDL_Window* window;

VkSurfaceKHR surface;
VkSemaphore imgAvailableSemaphore;
VkSemaphore renderDoneSemaphore;
VkFence frameCompleted;

VkRenderPass renderPass;
VkFramebuffer* framebuffers;
VkShaderModule vertModule, fragModule;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Vulkan", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);

	// Create an instance of vulkan
	createInstance("Vulkan");
	setupDebugging();

	SDL_CreateVulkanSurface(window, instance, &surface);
	printf("SDL errors: %s\n", SDL_GetError());

	getDevice();
	createSwapchain();

	// Create command buffers
	createCommandPool();
	VkCommandBuffer* comBuffers = calloc(
		sizeof(VkCommandBuffer), imageCount);
	for (int i = 0; i < imageCount; i++) {
		createCommandBuffer(&comBuffers[i]);
	}

	createGraphicsPipeline();

	setupFramebuffers();

	for (int i = 0; i < imageCount; i++) {
		beginCommands(&comBuffers[i]);

		transitionImage(swapImages[i], &comBuffers[i],
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0,
				VK_ACCESS_TRANSFER_WRITE_BIT);

		VkClearColorValue clearColor = {0};
		memcpy(clearColor.float32, (float[4]) {0, 0, 0.4, 1},
			sizeof(uint32_t[4]));

		VkImageSubresourceRange subresourceRange = {0};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;
		vkCmdClearColorImage(comBuffers[i], swapImages[i],
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor,
			1, &subresourceRange);

		transitionImage(swapImages[i], &comBuffers[i],
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

		beginRenderPass(comBuffers[i], framebuffers[i]);

		vkCmdBindPipeline(comBuffers[i],
			VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		vkCmdDraw(comBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(comBuffers[i]);

		transitionImage(swapImages[i], &comBuffers[i],
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_TRANSFER_READ_BIT);

		endCommands(&comBuffers[i]);
	}

	createSemaphore(&imgAvailableSemaphore);
	createSemaphore(&renderDoneSemaphore);
	createFence(&frameCompleted);

	double time = SDL_GetTicks() / 1000.0;
	int frames = 0, seconds = (int) time;
	while (!SDL_QuitRequested()) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
		}

		vkResetFences(device, 1, &frameCompleted);

		beginFrame();
		submitCommandBuffer(&comBuffers[imageIndex], frameCompleted);
		endFrame();

		vkWaitForFences(device, 1, &frameCompleted, VK_TRUE, ULONG_MAX);

		frames++;
		while ((int) time > seconds) {
			seconds++;
			printf("FPS: %d\n", frames);
			frames = 0;
		}
		time = SDL_GetTicks() / 1000.0;
	}

	destroySemaphore(&imgAvailableSemaphore);
	destroySemaphore(&renderDoneSemaphore);
	destroyFence(&frameCompleted);

	vkDestroyRenderPass(device, renderPass, NULL);
	for (int i = 0; i < imageCount; i++)
		vkDestroyFramebuffer(device, framebuffers[i], NULL);
	vkDestroyShaderModule(device, vertModule, NULL);
	vkDestroyShaderModule(device, fragModule, NULL);
	vkDestroyPipelineLayout(device, pipelineLayout, NULL);
	vkDestroyPipeline(device, graphicsPipeline, NULL);

	// DESTROY
	vkDestroyCommandPool(device, comPool, NULL);
	destroySwapchain();
	
	destroyDebugging();
	destroyInstance();

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

void createGraphicsPipeline() {
	// Compile the shaders
	int vertSize, fragSize;
	char* vertShader = loadFile("./shaders/vert.spv", &vertSize);
	char* fragShader = loadFile("./shaders/frag.spv", &fragSize);

	VkShaderModuleCreateInfo vertModuleInfo = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, NULL, 0,
		vertSize, (uint32_t*) vertShader
	};
	vkCreateShaderModule(device, &vertModuleInfo, NULL, &vertModule);

	VkShaderModuleCreateInfo fragModuleInfo = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, NULL, 0,
		fragSize, (uint32_t*) fragShader
	};
	vkCreateShaderModule(device, &fragModuleInfo, NULL, &fragModule);

	VkPipelineShaderStageCreateInfo vertCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, NULL, 0,
		VK_SHADER_STAGE_VERTEX_BIT, vertModule, "main", NULL
	};
	VkPipelineShaderStageCreateInfo fragCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, NULL, 0,
		VK_SHADER_STAGE_FRAGMENT_BIT, fragModule, "main", NULL
	};
	VkPipelineShaderStageCreateInfo shaderStages[2] = {
		fragCreateInfo, vertCreateInfo
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, NULL,
		0, 0, NULL, 0, NULL
	};

	// Declare what primitives to use when rendering
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		NULL, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE
	};

	// Set the viewport
	VkViewport viewport = {
		0, 0, width, height, 0, 1
	};

	VkRect2D scissor = {
		{0, 0}, {swapExtent.width, swapExtent.height}
	};

	VkPipelineViewportStateCreateInfo viewportInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, NULL,
		0, 1, &viewport, 1, &scissor
	};

	// Set rasterization options
	VkPipelineRasterizationStateCreateInfo rasterInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		NULL, 0, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE,
		VK_FALSE, 0, 0, 0, 1
	};

	// Create the multisampling structure
	VkPipelineMultisampleStateCreateInfo multisampling = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		NULL, 0, VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1, NULL,
		VK_FALSE, VK_FALSE
	};

	// Set the blend mode
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		VK_FALSE, 0, 0, 0, 0, 0, 0, VK_COLOR_COMPONENT_R_BIT
		| VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
		| VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo colorBlend = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		NULL, 0, VK_FALSE, 0, 1, &colorBlendAttachment, {0, 0, 0, 0}
	};

	// Create a pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, NULL, 0,
		0, NULL, 0, NULL
	};
	vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL,
		&pipelineLayout);

	// Create the renderpass
	VkAttachmentDescription attachment = {
		0, swapFormat, VK_SAMPLE_COUNT_1_BIT,
		VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkAttachmentReference reference = {
		0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass = {
		0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, NULL, 1, &reference,
		NULL, NULL, 0, NULL
	};

	VkRenderPassCreateInfo renderPassInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, NULL,
		0, 1, &attachment, 1, &subpass, 0, NULL
	};
	vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass);

	// Create the graphics pipeline!
	VkGraphicsPipelineCreateInfo pipelineInfo = {
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, NULL, 0,
		2, shaderStages, &vertexInputInfo, &inputAssemblyInfo, NULL,
		&viewportInfo, &rasterInfo, &multisampling, NULL, &colorBlend,
		NULL, pipelineLayout, renderPass, 0, VK_NULL_HANDLE, -1
	};

	vkCreateGraphicsPipelines(device, NULL, 1, &pipelineInfo,
		NULL, &graphicsPipeline);
}

void setupFramebuffers() {
	framebuffers = calloc(imageCount, sizeof(VkFramebuffer));

	for (int i = 0; i < imageCount; i++) {
		VkFramebufferCreateInfo framebufferInfo = {
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, NULL, 0,
			renderPass, 1, &swapViews[i], width, height, 1
		};
		vkCreateFramebuffer(device, &framebufferInfo, NULL,
			&framebuffers[i]);
	}
}

void beginRenderPass(VkCommandBuffer comBuffer, VkFramebuffer framebuffer) {
	VkRenderPassBeginInfo renderPassInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, NULL, renderPass,
		framebuffer, {{0, 0}, {width, height}}, 0, NULL
	};
	vkCmdBeginRenderPass(comBuffer, &renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE);
}

