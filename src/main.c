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
#include "util/window.h"

int main() {
	// Create an instance of vulkan
	createInstance("Vulkan");
	setupDebugging();

	getDevice();

	openWindow();

	createCommandPool();
	createCommandBuffer();

	prepRender();

	beginCommands();

	VkClearColorValue clearColor = {
		.uint32 = {1, 0, 0, 1}
	};

	VkImageMemoryBarrier preImageBarrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, NULL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL, queueFam,
		queueFam, swapImages[nextImage],
		swapViewInfos[nextImage].subresourceRange
	};

	vkCmdPipelineBarrier(
		comBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, NULL, 0, NULL, 1, &preImageBarrier
	);

	vkCmdClearColorImage(
		comBuffer, swapImages[nextImage], VK_IMAGE_LAYOUT_GENERAL,
		&clearColor, 1, &swapViewInfos[nextImage].subresourceRange
	);

	VkImageMemoryBarrier postImageBarrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, NULL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED, swapImages[nextImage],
		swapViewInfos[nextImage].subresourceRange
	};

	vkCmdPipelineBarrier(
		comBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, 
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
		0, 0, NULL, 0, NULL, 1, &postImageBarrier
	);

	endCommands();

	submitCommandBuffer();

	tickWindow();

	sleep(3);

	// DESTROY
	destroyInstance();

	quitWindow();
	return 0;
}

