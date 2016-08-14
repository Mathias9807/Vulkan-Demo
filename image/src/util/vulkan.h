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


#include <vulkan/vulkan.h>

extern VkInstance		instance;
extern VkDevice			device;
extern VkPhysicalDevice*	GPUs;
extern unsigned 		numGPUs;

extern int			queueFam;
extern VkQueue			queue;
extern VkCommandPool		comPool;
extern VkCommandBuffer		comBuffer;

void createInstance(char* name);
void setupDebugging();
void getDevice();
void createCommandPool();
void createCommandBuffer();
void beginCommands();
void endCommands();
void submitCommandBuffer();
void transitionImage(VkImage image,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		VkAccessFlags srcFlags, VkAccessFlags dstFlags);
void destroyDebugging();
void destroyInstance();
void check(VkResult r);


#endif // VULKAN_H
