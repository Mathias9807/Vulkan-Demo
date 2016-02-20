#include "util/vulkan.h"

VkInstance	instance;
VkDevice	device;

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

	vkCreateInstance(&createInfo, NULL, &instance);
}

void getDevice() {
	unsigned numGPUs = 0;
	VkPhysicalDevice GPUs[16]; // Future proof code
	
	vkEnumeratePhysicalDevices(instance, &numGPUs, GPUs);

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

	vkCreateDevice(
		GPUs[0], &devCreateInfo, NULL, &device
	);
}

void destroyInstance() {
	vkDestroyInstance(instance, NULL);
}

