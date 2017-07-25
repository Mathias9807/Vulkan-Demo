/*
 * Simple Vulkan application
 * 
 * Copyright (c) 2016 by Mathias Johansson
 * 
 * This code is licensed under the MIT license 
 * 		https://opensource.org/licenses/MIT
 */

#ifndef VULKAN_H
#define VULKAN_H


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

extern VkInstance		instance;
extern VkDevice			device;
extern VkPhysicalDevice*	GPUs;
extern unsigned 		numGPUs;

extern int			queueFam;
extern VkQueue			queue;
extern VkCommandPool		comPool;

extern VkSwapchainKHR		swap;
extern VkFormat			swapFormat;
extern VkExtent2D		swapExtent;
extern VkImage*			swapImages;
extern VkImageView*		swapViews;
extern VkImageViewCreateInfo*	swapViewInfos;
extern unsigned			imageCount;
extern unsigned			imageIndex;

void createInstance(char* name);
void setupDebugging();
void getDevice();
void createSwapchain();
void createCommandPool();
void createCommandBuffer(VkCommandBuffer* comBuffer);
void createSemaphore(VkSemaphore* semaphore);
void createFence(VkFence* fence);

void beginFrame();
void endFrame();
void beginCommands(VkCommandBuffer* comBuffer);
void endCommands(VkCommandBuffer* comBuffer);
void submitCommandBuffer(VkCommandBuffer* comBuffer, VkFence fence);
void transitionImage(VkImage image, VkCommandBuffer* comBuffer,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		VkAccessFlags srcFlags, VkAccessFlags dstFlags);

void destroySemaphore(VkSemaphore* semaphore);
void destroyFence(VkFence* fence);
void destroySwapchain();
void destroyDebugging();
void destroyInstance();
void check(VkResult r);


#endif // VULKAN_H
