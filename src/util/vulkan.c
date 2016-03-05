/*
 * Simple Vulkan application
 * 
 * Copyright (c) 2016 by Mathias Johansson
 * 
 * This code is licensed under the MIT license 
 * 		https://opensource.org/licenses/MIT
 */

#include "util/vulkan.h"
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VkInstance		instance;
VkDevice		device;
VkPhysicalDevice GPUs[16]; // Future proof code
unsigned 		numGPUs = 0;

VkQueue			queue;
VkCommandPool	comPool;
VkCommandBuffer	comBuffer;

VkSwapchainKHR	swap;
VkImage			swapImages[4];
VkImageViewCreateInfo		swapViews[4];
unsigned		swapCount;

float queuePriorities = 0;

void createInstance(char* name) {
	VkApplicationInfo appInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO, 
		NULL, name, 1, name, 1, VK_MAKE_VERSION(1, 0, 1)
	};

	unsigned numExts = 8;
	VkExtensionProperties exts[8];
	memset(exts, 0, sizeof(exts));
	vkEnumerateInstanceExtensionProperties(NULL, &numExts, exts);

	const char* used[3];
	used[0] = VK_KHR_SURFACE_EXTENSION_NAME;
	used[1] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
	used[2] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
	
	VkInstanceCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, 
		NULL, 0, &appInfo, 0, NULL, 
		3, used
	};

	check(vkCreateInstance(&createInfo, NULL, &instance));
}

void getDevice() {
	// List all graphics and compute devices
	check(vkEnumeratePhysicalDevices(instance, &numGPUs, GPUs));

	// Pretend to look at the actual features
	VkPhysicalDeviceFeatures pDevFeatures;
	vkGetPhysicalDeviceFeatures(GPUs[0], &pDevFeatures);

	// Query the device's available extensions
	VkExtensionProperties exts[8];
	memset(exts, 0, sizeof(exts));
	unsigned numExts = 8;
	vkEnumerateDeviceExtensionProperties(GPUs[0], NULL, &numExts, exts);

	// Ignore them and just hardcode it instead
	const char* used[1];
	used[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

	// Get queue family properties
	unsigned numQFams = 16;
	VkQueueFamilyProperties famProps[16];
	vkGetPhysicalDeviceQueueFamilyProperties(
		GPUs[0], &numQFams, famProps
	);

	// Search for a graphics queue family
	unsigned graphicsQueueFam = 0;
	for (int i = 0; i < numQFams; i++) 
		if (famProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsQueueFam = i;
			break;
		}

	VkDeviceQueueCreateInfo devQueueCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, 
		NULL, 0, graphicsQueueFam, 1, &queuePriorities
	};

	VkDeviceCreateInfo devCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, 
		NULL, 0, 1, &devQueueCreateInfo, 0, NULL, 
		1, used, &pDevFeatures
	};

	// Create the logical device
	check(vkCreateDevice(
		GPUs[0], &devCreateInfo, NULL, &device
	));

	// Get the queue
	vkGetDeviceQueue(device, graphicsQueueFam, 0, &queue);

	// Wait for completion, we've got time
	vkDeviceWaitIdle(device);
}

void createCommandPool() {
	VkCommandPoolCreateInfo comPoolCreate = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, 
		NULL, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, 0
	};
	
	check(vkCreateCommandPool(device, &comPoolCreate, NULL, &comPool));

	vkDeviceWaitIdle(device);
}

void createCommandBuffer() {
	VkCommandBufferAllocateInfo comBufferInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, 
		NULL, comPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1
	};

	check(vkAllocateCommandBuffers(device, &comBufferInfo, &comBuffer));

	vkDeviceWaitIdle(device);
}

void beginCommands() {
	VkCommandBufferBeginInfo beginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 
		NULL, 0, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, 
		VK_TRUE, VK_NULL_HANDLE, VK_NULL_HANDLE
	};
	
	check(vkBeginCommandBuffer(comBuffer, &beginInfo));

	vkDeviceWaitIdle(device);
}

void endCommands() {
	check(vkEndCommandBuffer(comBuffer));

	vkDeviceWaitIdle(device);
}

void submitCommandBuffer() {
	VkPipelineStageFlags flag = 0;
	VkSubmitInfo submitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO, 
		NULL, 0, NULL, &flag, 1, &comBuffer, 0, NULL
	};

	check(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

	vkDeviceWaitIdle(device);
}

void destroyInstance() {
	vkDestroyInstance(instance, NULL);
}

void check(VkResult r) {
	if (r != VK_SUCCESS) {
		printf("Error on Vulkan call\n");
		exit(1);
	}
}

