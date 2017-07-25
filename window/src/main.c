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

int width = 512, height = 512;
SDL_Window* window;

VkSurfaceKHR surface;
VkSemaphore imgAvailableSemaphore;
VkSemaphore renderDoneSemaphore;
VkFence frameCompleted;

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

	createCommandPool();
	VkCommandBuffer* comBuffers = calloc(
		sizeof(VkCommandBuffer), imageCount);

	for (int i = 0; i < imageCount; i++) {
		createCommandBuffer(&comBuffers[i]);

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
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_ACCESS_TRANSFER_WRITE_BIT,
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

	// Clear the image
	// beginCommands();

	// transitionImage(image, VK_IMAGE_LAYOUT_UNDEFINED,
	// 		VK_IMAGE_LAYOUT_GENERAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT);

	// VkClearColorValue clearColor = {0};
	// memcpy(clearColor.float32, (float[4]) {0, 0, 0.4, 1}, sizeof(uint32_t[4]));
	// VkImageSubresourceRange subresourceRange = {0};
	// subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	// subresourceRange.baseMipLevel = 0;
	// subresourceRange.levelCount = 1;
	// subresourceRange.baseArrayLayer = 0;
	// subresourceRange.layerCount = 1;
	// vkCmdClearColorImage(comBuffer, image, VK_IMAGE_LAYOUT_GENERAL, &clearColor,
	// 	1, &subresourceRange);

	// transitionImage(image, VK_IMAGE_LAYOUT_GENERAL,
	// 		VK_IMAGE_LAYOUT_GENERAL,
	// 		VK_ACCESS_HOST_READ_BIT | VK_ACCESS_MEMORY_READ_BIT
	// 		| VK_ACCESS_TRANSFER_WRITE_BIT,
	// 		VK_ACCESS_TRANSFER_READ_BIT);

	// endCommands();

	// submitCommandBuffer();

	// DESTROY
	vkDestroyCommandPool(device, comPool, NULL);
	destroySwapchain();
	
	destroyDebugging();
	destroyInstance();

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

