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

#include "util/vulkan.h"

void createGraphicsPipeline();

int width = 512, height = 512;

VkImageCreateInfo imageInfo;

VkRenderPass renderPass;
VkShaderModule vertModule, fragModule;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;

int main() {
	// Create an instance of vulkan
	createInstance("Vulkan");
	setupDebugging();

	getDevice();

	createCommandPool();
	createCommandBuffer();

	createGraphicsPipeline();

	// Create an image
	VkImage image;
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT
		| VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	vkCreateImage(device, &imageInfo, NULL, &image);

	// Allocate memory for it
	unsigned dataSize = width * height * 4;
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {-1};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = 0;

	VkDeviceMemory imageMemory;
	vkAllocateMemory(device, &allocInfo, NULL, &imageMemory);
	vkBindImageMemory(device, image, imageMemory, 0);

	// Clear the image
	beginCommands();

	transitionImage(image, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_GENERAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT);

	VkClearColorValue clearColor = {-1};
	memcpy(clearColor.float32, (float[4]) {0, 0, 0.4, 1}, sizeof(uint32_t[4]));
	VkImageSubresourceRange subresourceRange = {0};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;
	vkCmdClearColorImage(comBuffer, image, VK_IMAGE_LAYOUT_GENERAL, &clearColor,
		1, &subresourceRange);

	transitionImage(image, VK_IMAGE_LAYOUT_GENERAL,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_ACCESS_HOST_READ_BIT | VK_ACCESS_MEMORY_READ_BIT
			| VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_TRANSFER_READ_BIT);

	endCommands();

	submitCommandBuffer();

	// Print image data
	FILE* img = fopen("image.ppm", "wb");
	fprintf(img, "P6\n%d %d\n255\n", width, height);

	char* data;
	vkMapMemory(device, imageMemory, 0, memRequirements.size, 0, (void*) &data);
	for (int i = 0; i < dataSize - sizeof(char) * 4; i++) {
		if ((i + 1) % 4 == 0) continue;
		fputc(data[i], img);
	}
	fputc(255, img);
	fputc(0, img);
	fputc(0, img);
	fputc(0, img);
	vkUnmapMemory(device, imageMemory);

	fclose(img);

	// DESTROY
	vkDestroyCommandPool(device, comPool, NULL);
	vkDestroyImage(device, image, NULL);
	vkFreeMemory(device, imageMemory, NULL);

	vkDestroyRenderPass(device, renderPass, NULL);
	vkDestroyShaderModule(device, vertModule, NULL);
	vkDestroyShaderModule(device, fragModule, NULL);
	vkDestroyPipelineLayout(device, pipelineLayout, NULL);
	vkDestroyPipeline(device, graphicsPipeline, NULL);
	
	destroyDebugging();
	destroyInstance();
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
		{0, 0}, {imageInfo.extent.width, imageInfo.extent.height}
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
		0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT,
		VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL
	};

	VkAttachmentReference reference = {
		0, VK_IMAGE_LAYOUT_GENERAL
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

