#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "util/vulkan.h"
#include "util/window.h"

int main() {
	// Create an instance of vulkan
	createInstance("Vulkan");

	getDevice();

	openWindow();

	createCommandPool();
	createCommandBuffer();

	prepRender();

	beginCommands();
	VkClearColorValue clearColor = {
		.int32 = {0, 127, INT32_MAX}
	};
	vkCmdClearColorImage(
		comBuffer, swapImages[nextImage], VK_IMAGE_LAYOUT_GENERAL, 
		&clearColor, 1, &swapViews[nextImage].subresourceRange
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

