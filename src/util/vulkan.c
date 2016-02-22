#include "util/vulkan.h"
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>

VkInstance		instance;
VkDevice		device;

VkQueue			queue;
VkCommandPool	comPool;
VkCommandBuffer	comBuffer;

VkSwapchainKHR	swap;
VkImage			swapImages[4];
VkImageViewCreateInfo		swapViews[4];
unsigned		swapCount;

float queuePriorities = 0; // I don't really know

const char* extensions[] = {
	VK_KHR_SURFACE_EXTENSION_NAME, 
};

void createInstance(char* name) {
	VkApplicationInfo appInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO, 
		NULL, name, 1, name, 1, VK_MAKE_VERSION(1, 0, 0)
	};
	
	VkInstanceCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, 
		NULL, 0, &appInfo, 0, NULL, 
		sizeof(extensions) / sizeof(extensions[0]), 
		extensions
	};

	check(vkCreateInstance(&createInfo, NULL, &instance));
}

void getDevice() {
	unsigned numGPUs = 0;
	VkPhysicalDevice GPUs[16]; // Future proof code
	
	check(vkEnumeratePhysicalDevices(instance, &numGPUs, GPUs));

	VkPhysicalDeviceFeatures pDevFeatures;
	vkGetPhysicalDeviceFeatures(GPUs[0], &pDevFeatures);

	VkDeviceQueueCreateInfo devQueueCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, 
		NULL, 0, 0, 1, &queuePriorities
	};

	VkDeviceCreateInfo devCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, 
		NULL, 0, 1, &devQueueCreateInfo, 0, NULL, 0, NULL, 
		&pDevFeatures
	};

	check(vkCreateDevice(
		GPUs[0], &devCreateInfo, NULL, &device
	));

	// Get the queue
	vkGetDeviceQueue(device, 0, 0, &queue);
}

void createCommandPool() {
	VkCommandPoolCreateInfo comPoolCreate = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, 
		NULL, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, 0
	};
	
	check(vkCreateCommandPool(device, &comPoolCreate, NULL, &comPool));
}

void createCommandBuffer() {
	VkCommandBufferAllocateInfo comBufferInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, 
		NULL, comPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1
	};

	check(vkAllocateCommandBuffers(device, &comBufferInfo, &comBuffer));
}

void beginCommands() {
	VkCommandBufferBeginInfo beginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 
		NULL, 0, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, 
		VK_TRUE, VK_NULL_HANDLE, VK_NULL_HANDLE
	};
	
	check(vkBeginCommandBuffer(comBuffer, &beginInfo));
}

void endCommands() {
	check(vkEndCommandBuffer(comBuffer));
}

void submitCommandBuffer() {
	VkPipelineStageFlags flag = 0;
	VkSubmitInfo submitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO, 
		NULL, 0, NULL, &flag, 1, &comBuffer, 0, NULL
	};

	check(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
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

