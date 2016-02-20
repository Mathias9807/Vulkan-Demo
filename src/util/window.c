#include <stdio.h>
#include <malloc.h>
#include "window.h"

// Windowing
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// VULKAN
#include <vulkan/vulkan.h>

bool isClosing = false;
VkSurfaceKHR surface;

void openWindow() {
	// Get Vulkan surface
	/*VkSwapchainCreateInfoKHR swapCreateInfo = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, 
		NULL, 0, surface, 1, VK_FORMAT_R8G8B8_UNORM, 
		VK_COLORSPACE_SRGB_NONLINEAR_KHR, {800, 600}, 
		1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
		VK_SHARING_MODE_EXCLUSIVE, 0, NULL, 0, 0, 
		VK_PRESENT_MODE_IMMEDIATE_KHR, 
	};
	VkSwapchainKHR swap;
	vkCreateSwapchainKHR(dev, &swapCreateInfo, NULL, &swap);

	// Again this should be properly enumerated
	VkImage images[4]; uint32_t swapCount;
	vkGetSwapchainImagesKHR(dev, swap, &swapCount, images);

	// Synchronisation is needed here!
	uint32_t currentSwapImage;
	vkAcquireNextImageKHR(dev, swap, UINT64_MAX, presentCompleteSemaphore, NULL, &currentSwapImage);*/
}

void tickWindow() {
}

void quitWindow() {
}

