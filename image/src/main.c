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

int width = 16, height = 16;

int main() {
	// Create an instance of vulkan
	createInstance("Vulkan");
	setupDebugging();

	getDevice();

	createCommandPool();
	createCommandBuffer();

	// Create an image
	VkImage image;
	VkImageCreateInfo imageInfo = {0};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8_UNORM;
	imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	vkCreateImage(device, &imageInfo, NULL, &image);

	// Allocate memory for it
	unsigned dataSize = width * height * 3;
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {0};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = 0;

	VkDeviceMemory imageMemory;
	vkAllocateMemory(device, &allocInfo, NULL, &imageMemory);
	vkBindImageMemory(device, image, imageMemory, 0);

	// Clear the image
	beginCommands();

	transitionImage(image, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	VkClearColorValue clearColor = {0};
	memcpy(clearColor.uint32, (uint32_t[4]) {0, 0, 0xFFFFFFFF, 0xFFFFFFFF},
		sizeof(uint32_t[4]));
	memset(&clearColor, 255, sizeof(clearColor));
	VkImageSubresourceRange subresourceRange = {0};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;
	vkCmdClearColorImage(comBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, &clearColor,
		1, &subresourceRange);

	transitionImage(image, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	endCommands();

	submitCommandBuffer();

	// Print image data
	FILE* img = fopen("image.ppm", "wb");
	fprintf(img, "P6\n%d %d\n255\n", width, height);

	void* data;
	vkMapMemory(device, imageMemory, 0, memRequirements.size, 0, &data);
	fwrite(data, dataSize - sizeof(char[3]), 1, img);
	fputc(255, img);
	fputc(0, img);
	fputc(0, img);
	vkUnmapMemory(device, imageMemory);

	fclose(img);

	// DESTROY
	vkDestroyCommandPool(device, comPool, NULL);
	vkDestroyImage(device, image, NULL);
	vkFreeMemory(device, imageMemory, NULL);
	
	destroyDebugging();
	destroyInstance();
	return 0;
}

