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

	VkClearColorValue clearColor = {0};
	memset(&clearColor, 1, sizeof(VkClearColorValue));

	vkCmdPipelineBarrier(
		comBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, NULL, 0, NULL, 0, NULL
	);

	vkCmdClearColorImage(
		comBuffer, swapImages[nextImage], VK_IMAGE_LAYOUT_GENERAL,
		&clearColor, 1, &swapViews[nextImage].subresourceRange
	);

	vkCmdPipelineBarrier(
		comBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, 
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
		0, 0, NULL, 0, NULL, 0, NULL
	);

	endCommands();

	submitCommandBuffer();

	vkDeviceWaitIdle(device);

	tickWindow();

	sleep(3);

	// DESTROY
	destroyInstance();

	quitWindow();
	return 0;
}

