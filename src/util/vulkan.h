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


#include "window.h"

#ifdef XCB_WINDOWING
#define VK_USE_PLATFORM_XCB_KHR
#elif defined(X11_WINDOWING)
#define VK_USE_PLATFORM_XLIB_KHR
#else
#error "Define XCB_WINDOWING or X11_WINDOWING"
#endif

#include <vulkan/vulkan.h>

extern VkInstance		instance;
extern VkDevice			device;
extern VkPhysicalDevice*	GPUs;
extern unsigned 		numGPUs;

extern int			queueFam;
extern VkQueue			queue;
extern VkCommandPool		comPool;
extern VkCommandBuffer		comBuffer;

extern VkSwapchainKHR		swap;
extern VkSemaphore		preRenderSem;
extern VkImage*			swapImages;
extern VkImageView*		swapViews;
extern VkImageViewCreateInfo*	swapViewInfos;
extern unsigned			nextImage;
extern unsigned			swapCount;

void createInstance(char* name);
void setupDebugging();
void getDevice();
void createCommandPool();
void createCommandBuffer();
void beginCommands();
void endCommands();
void submitCommandBuffer();
void destroyInstance();
void check(VkResult r);


#endif // VULKAN_H
