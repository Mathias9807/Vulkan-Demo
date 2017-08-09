/*
 * Simple Vulkan application
 * 
 * Copyright (c) 2016 by Mathias Johansson
 * 
 * This code is licensed under the MIT license 
 * 		https://opensource.org/licenses/MIT
 */

#include "util/vulkan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VkInstance		instance;
char			layers[256][16];
char			extensions[256][16];

VkDebugReportCallbackEXT callback;

VkDevice		device;
VkPhysicalDevice*	GPUs;
unsigned 		numGPUs = 0;

extern VkSurfaceKHR	surface;

int			queueFam;
VkQueue			queue;
VkCommandPool		comPool;
extern VkCommandBuffer* comBuffers;

VkSwapchainKHR		swap;
VkFormat		swapFormat;
VkExtent2D		swapExtent;
VkImage*		swapImages;
VkImageView*		swapViews;
VkImageViewCreateInfo*	swapViewInfos;
unsigned		imageCount;
unsigned		imageIndex = -1;

extern VkSemaphore	imgAvailableSemaphore;
extern VkSemaphore	renderDoneSemaphore;

float			queuePriorities = 1;

void createInstance(char* name) {
	VkApplicationInfo appInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO, 
		NULL, name, 1, name, 1, VK_MAKE_VERSION(1, 0, 1)
	};

	// Enumerate all available layers
	unsigned numLayers = 64;
	VkLayerProperties foundLayers[64] = {0};
	check(vkEnumerateInstanceLayerProperties(&numLayers, foundLayers));
	printf("Found %d layers:\n", numLayers);
	for (int i = 0; i < numLayers; i++)
		printf("\t%s", foundLayers[i].layerName);
	printf("\n\n");

	unsigned numExts = 16;
	VkExtensionProperties exts[16];
	memset(exts, 0, sizeof(exts));
	check(vkEnumerateInstanceExtensionProperties(NULL, &numExts, exts));
	printf("Found %d extensions:\n", numExts);
	for (int i = 0; i < numExts; i++)
		printf("%s\t", exts[i].extensionName);
	printf("\n\n");

	const char* usedExts[numExts];
	int extI = 0;
	for (int i = 0; i < numExts; i++) {
		if (!strcmp(exts[i].extensionName, 
				"VK_EXT_debug_report")) {
			usedExts[extI++] = exts[i].extensionName;
			printf("Uses extension %s\n", exts[i].extensionName);
			continue;
		}
		if (!strcmp(exts[i].extensionName, 
				"VK_KHR_surface")) {
			usedExts[extI++] = exts[i].extensionName;
			printf("Uses extension %s\n", exts[i].extensionName);
			continue;
		}
		if (!strcmp(exts[i].extensionName, 
				"VK_KHR_xcb_surface")) {
			usedExts[extI++] = exts[i].extensionName;
			printf("Uses extension %s\n", exts[i].extensionName);
			continue;
		}
	}
	for (int i = 0; i < extI; i++) 
		memcpy(extensions[i], usedExts[i], 256);

	// Use layers if available
	const char* usedLayers[numLayers];
	int j = 0;
	for (int i = 0; i < numLayers; i++) {
		if (!strcmp(foundLayers[i].layerName, 
				"VK_LAYER_LUNARG_parameter_validation")) {
			usedLayers[j++] = foundLayers[i].layerName;
			printf("Uses layer %s\n", foundLayers[i].layerName);
		}
		if (!strcmp(foundLayers[i].layerName, 
				"VK_LAYER_LUNARG_object_tracker")) {
			usedLayers[j++] = foundLayers[i].layerName;
			printf("Uses layer %s\n", foundLayers[i].layerName);
		}
		if (!strcmp(foundLayers[i].layerName, 
				"VK_LAYER_LUNARG_core_validation")) {
			usedLayers[j++] = foundLayers[i].layerName;
			printf("Uses layer %s\n", foundLayers[i].layerName);
		}
		if (!strcmp(foundLayers[i].layerName, 
				"VK_LAYER_LUNARG_standard_validation")) {
			usedLayers[j++] = foundLayers[i].layerName;
			printf("Uses layer %s\n", foundLayers[i].layerName);
		}
	}
	for (int i = 0; i < j; i++) 
		memcpy(layers[i], usedLayers[i], 256);
	
	VkInstanceCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, 
		NULL, 0, &appInfo, j, usedLayers, 
		extI, usedExts
	};

	check(vkCreateInstance(&createInfo, NULL, &instance));
}

void getDevice() {
	// List all graphics and compute devices
	// First get number of devices
	check(vkEnumeratePhysicalDevices(instance, &numGPUs, NULL));
	printf("Found %d physical device[s]\n", numGPUs);
	
	// Then get the device handles themselves
	GPUs = calloc(numGPUs, sizeof(VkPhysicalDevice));
	check(vkEnumeratePhysicalDevices(instance, &numGPUs, GPUs));

	// Pretend to look at the actual features
	VkPhysicalDeviceFeatures pDevFeatures;
	vkGetPhysicalDeviceFeatures(GPUs[0], &pDevFeatures);

	// Check device memory heaps
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(GPUs[0], &memProps);
	printf("Found %d device heap[s]\n", memProps.memoryHeapCount);
	for (int i = 0; i < memProps.memoryHeapCount; i++)
		printf("\tFound %d kiB large device heap\n",
			(int) memProps.memoryHeaps[i].size / 1024);

	// Query the device's available layers
	unsigned numLayers = 16;
	VkLayerProperties foundLayers[16];
	memset(foundLayers, 0, sizeof(foundLayers));
	check(vkEnumerateDeviceLayerProperties(GPUs[0], &numLayers, foundLayers));
	printf("Device has %d layers\n", numLayers);

	// Query the device's available extensions
	VkExtensionProperties exts[8];
	memset(exts, 0, sizeof(exts));
	unsigned numExts = 8;
	check(vkEnumerateDeviceExtensionProperties(GPUs[0], NULL, &numExts, exts));
	printf("Device has %d extensions\n", numExts);

	// Print available extensions
	for (int i = 0; i < numExts; i++)
		printf("%s\n", exts[i].extensionName);

	const char* usedExts[numExts];
	int extI = 0;
	for (int i = 0; i < numExts; i++) {
		if (!strcmp(exts[i].extensionName, 
				"VK_EXT_debug_report")) {
			usedExts[extI++] = exts[i].extensionName;
			printf("Uses device extension %s\n", exts[i].extensionName);
			continue;
		}
		if (!strcmp(exts[i].extensionName, 
				"VK_KHR_surface")) {
			usedExts[extI++] = exts[i].extensionName;
			printf("Uses device extension %s\n", exts[i].extensionName);
			continue;
		}
		if (!strcmp(exts[i].extensionName, 
				"VK_KHR_xcb_surface")) {
			usedExts[extI++] = exts[i].extensionName;
			printf("Uses device extension %s\n", exts[i].extensionName);
			continue;
		}
		if (!strcmp(exts[i].extensionName, 
				"VK_KHR_swapchain")) {
			usedExts[extI++] = exts[i].extensionName;
			printf("Uses device extension %s\n", exts[i].extensionName);
			continue;
		}
	}

	const char* usedLayers[numLayers];
	int j = 0;
	for (int i = 0; i < numLayers; i++) {
		if (!strcmp(foundLayers[i].layerName, 
				"VK_LAYER_LUNARG_parameter_validation")) {
			usedLayers[j++] = foundLayers[i].layerName;
			printf("Uses device layer %s\n", foundLayers[i].layerName);
		}
		if (!strcmp(foundLayers[i].layerName, 
				"VK_LAYER_LUNARG_object_tracker")) {
			usedLayers[j++] = foundLayers[i].layerName;
			printf("Uses device layer %s\n", foundLayers[i].layerName);
		}
		if (!strcmp(foundLayers[i].layerName, 
				"VK_LAYER_LUNARG_core_validation")) {
			usedLayers[j++] = foundLayers[i].layerName;
			printf("Uses device layer %s\n", foundLayers[i].layerName);
		}
		if (!strcmp(foundLayers[i].layerName, 
				"VK_LAYER_LUNARG_standard_validation")) {
			usedLayers[j++] = foundLayers[i].layerName;
			printf("Uses device layer %s\n", foundLayers[i].layerName);
		}
	}

	// Get queue family properties
	unsigned numQFams;
	vkGetPhysicalDeviceQueueFamilyProperties(GPUs[0], &numQFams, NULL);

	VkQueueFamilyProperties famProps[numQFams];
	vkGetPhysicalDeviceQueueFamilyProperties(
		GPUs[0], &numQFams, famProps
	);

	// Search for a graphics queue family
	queueFam = 0;
	for (int i = 0; i < numQFams; i++) {
		bool support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(GPUs[0], i,
			surface, (VkBool32*) &support);
		printf("Queue support for surface = %s\n", support ?
			"true" : "false");

		if (support && famProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			queueFam = i;
			break;
		}
	}

	VkDeviceQueueCreateInfo devQueueCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, 
		NULL, 0, queueFam, 1, &queuePriorities
	};

	VkDeviceCreateInfo devCreateInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, 
		NULL, 0, 1, &devQueueCreateInfo, j, usedLayers, 
		extI, usedExts, &pDevFeatures
	};

	// Create the logical device
	check(vkCreateDevice(
		GPUs[0], &devCreateInfo, NULL, &device
	));

	// Get the queue
	vkGetDeviceQueue(device, queueFam, 0, &queue);

	// Wait for completion, we've got time
	check(vkDeviceWaitIdle(device));
}

void createSwapchain() {
	// Get all supported formats
	unsigned numFormats;
	vkGetPhysicalDeviceSurfaceFormatsKHR(GPUs[0], surface,
		&numFormats, NULL);

	VkSurfaceFormatKHR formats[numFormats];
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		GPUs[0], surface, &numFormats, formats
	);

	// Select a format
	VkSurfaceFormatKHR* format = NULL;
	for (int i = 0; i < numFormats; i++) {
		if (formats[i].format == VK_FORMAT_R8G8B8_UINT
				|| formats[i].colorSpace
				== VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			format = &formats[i];
			break;
		}
	}
	assert(format != NULL);
	swapFormat = format->format;

	// Get surface capabilities
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GPUs[0], surface, &capabilities);
	printf("Surface requires at least %d image(s) and at most %d image(s)\n", 
		capabilities.minImageCount, capabilities.maxImageCount);

	// Is surface presentation supported?
	VkBool32 supported;
	vkGetPhysicalDeviceSurfaceSupportKHR(GPUs[0], queueFam, surface, &supported);
	if (supported)
		printf("Surface presentation is supported by this device\n");
	else
		printf("Surface presentation is NOT supported by this device\n");

	// Get present modes
	unsigned numPresentModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		GPUs[0], surface, &numPresentModes, NULL);

	VkPresentModeKHR presentModes[numPresentModes];
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		GPUs[0], surface, &numPresentModes, presentModes);

	// Select a present mode
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (int i = 0; i < numPresentModes; i++)
		if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			presentMode = presentModes[i];
			printf("Using an IMMEDIATE present mode (No VSync)\n");
			break;
		}

	printf("MinExtent = %d, %d\n", capabilities.minImageExtent.width,
		capabilities.minImageExtent.height);
	printf("MaxExtent = %d, %d\n", capabilities.maxImageExtent.width,
		capabilities.maxImageExtent.height);
	printf("CurrentExtent = %d, %d\n", capabilities.currentExtent.width,
		capabilities.currentExtent.height);
	swapExtent = capabilities.currentExtent;

	imageCount = capabilities.minImageCount + 1;
	printf("Creating a swapchain of length %d\n", imageCount);

	// Create a swapchain
	VkSwapchainCreateInfoKHR swapCreateInfo = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, NULL, 0,
		surface, imageCount, format->format, 
		format->colorSpace, capabilities.currentExtent, 
		1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT, 
		VK_SHARING_MODE_EXCLUSIVE, 0, NULL, 
		capabilities.currentTransform, 
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, 
		presentMode, true, VK_NULL_HANDLE
	};
	vkCreateSwapchainKHR(device, &swapCreateInfo, NULL, &swap);

	// Get the swapchains images
	vkGetSwapchainImagesKHR(device, swap, &imageCount, NULL);
	printf("Number of images in swapchain: %d\n", imageCount);

	swapImages = calloc(sizeof(VkImage), imageCount);
	vkGetSwapchainImagesKHR(device, swap, &imageCount, swapImages);
	assert(swapImages != NULL);

	// Create imageviews
	swapViews = calloc(sizeof(VkImageView), imageCount);
	swapViewInfos = calloc(sizeof(VkImageViewCreateInfo), imageCount);
	for (int i = 0; i < imageCount; i++) {
		swapViewInfos[i] = (VkImageViewCreateInfo) {
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, NULL,
			0, swapImages[i], VK_IMAGE_VIEW_TYPE_2D, swapFormat, {
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY
			}, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
		};
		vkCreateImageView(device, &swapViewInfos[i], NULL, &swapViews[i]);
	}
}

void beginFrame() {
	vkAcquireNextImageKHR(device, swap, 1000000000UL, imgAvailableSemaphore,
		VK_NULL_HANDLE, &imageIndex);
}

void endFrame() {
	VkPresentInfoKHR presentInfo = {
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, NULL, 1,
		&renderDoneSemaphore, 1, &swap, &imageIndex, NULL
	};
	vkQueuePresentKHR(queue, &presentInfo);
}

void createSemaphore(VkSemaphore* semaphore) {
	VkSemaphoreCreateInfo semaphoreInfo = {0};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(device, &semaphoreInfo, NULL, semaphore);
}

void destroySemaphore(VkSemaphore* semaphore) {
	vkDestroySemaphore(device, *semaphore, NULL);
}

void createFence(VkFence* fence) {
	VkFenceCreateInfo fenceInfo = {0};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(device, &fenceInfo, NULL, fence);
}

void destroyFence(VkFence* fence) {
	vkDestroyFence(device, *fence, NULL);
}

static VkBool32 debugCallback(VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location,
	int32_t code, const char* layerPrefix, const char* msg, void* userData) {
	printf("%s\n", msg);
	return VK_FALSE;
}

void setupDebugging() {
	VkDebugReportCallbackCreateInfoEXT createInfo = {
		VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT, 0,
		VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT
			| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
			| VK_DEBUG_REPORT_INFORMATION_BIT_EXT,
		(PFN_vkDebugReportCallbackEXT) debugCallback, NULL};

	PFN_vkCreateDebugReportCallbackEXT f = (PFN_vkCreateDebugReportCallbackEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

	if (f != NULL) (*f)(instance, &createInfo, NULL, &callback);
}

void createCommandPool() {
	VkCommandPoolCreateInfo comPoolCreate = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, 
		NULL, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, 0
	};
	
	check(vkCreateCommandPool(device, &comPoolCreate, NULL, &comPool));

	check(vkDeviceWaitIdle(device));
}

void createCommandBuffer(VkCommandBuffer* comBuffer) {
	VkCommandBufferAllocateInfo comBufferInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, 
		NULL, comPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1
	};

	check(vkAllocateCommandBuffers(device, &comBufferInfo, comBuffer));

	check(vkDeviceWaitIdle(device));
}

void beginCommands(VkCommandBuffer* comBuffer) {
	VkCommandBufferBeginInfo beginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 
		NULL, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, NULL
	};
	
	check(vkBeginCommandBuffer(*comBuffer, &beginInfo));
}

void endCommands(VkCommandBuffer* comBuffer) {
	check(vkEndCommandBuffer(*comBuffer));
}

void submitCommandBuffer(VkCommandBuffer* comBuffer, VkFence fence) {
	VkPipelineStageFlags flag = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	VkSubmitInfo submitInfo = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 1, &imgAvailableSemaphore,
		&flag, 1, comBuffer, 1, &renderDoneSemaphore
	};

	vkQueueSubmit(queue, 1, &submitInfo, fence);
}

void transitionImage(VkImage image, VkCommandBuffer* comBuffer,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		VkAccessFlags srcFlags, VkAccessFlags dstFlags) {
	VkImageMemoryBarrier barrier = {0};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = srcFlags;
	barrier.dstAccessMask = dstFlags;
	vkCmdPipelineBarrier(
		*comBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0, 0, NULL, 0, NULL, 1, &barrier
	);
}

void destroySwapchain() {
	for (int i = 0; i < imageCount; i++) {
		vkDestroyImageView(device, swapViews[i], NULL);
	}

	vkDestroySwapchainKHR(device, swap, NULL);
}

void destroyDebugging() {
	PFN_vkDestroyDebugReportCallbackEXT f = (PFN_vkDestroyDebugReportCallbackEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

	if (f != NULL) (*f)(instance, callback, NULL);
	printf("Removed debugging callbacks\n");
}

void destroyInstance() {
	vkDestroyDevice(device, NULL);
	vkDestroyInstance(instance, NULL);
	printf("Closed vulkan instance\n");
}

char* loadFile(const char* path, int* fileSize) {
	FILE* file = fopen(path, "rb");

	fseek(file, 0L, SEEK_END);
	int size = ftell(file);
	rewind(file);

	char* data = calloc(size, 1);
	fread(data, sizeof(char), size, file);

	*fileSize = size;
	return data;
}

void check(VkResult r) {
	if (r != VK_SUCCESS) {
		printf("Error on Vulkan call\n");
		exit(1);
	}
}

