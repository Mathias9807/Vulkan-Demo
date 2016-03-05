/*
 * Simple Vulkan application
 * 
 * Copyright (c) 2016 by Mathias Johansson
 * 
 * This code is licensed under the MIT license 
 * 		https://opensource.org/licenses/MIT
 */

#include <stdio.h>
#include <malloc.h>
#include "window.h"

// Windowing
#include <xcb/xcb.h>

// VULKAN
#include "util/vulkan.h"

bool isClosing = false;
VkSurfaceKHR surface;
unsigned nextImage = 0;

xcb_connection_t* con;
xcb_screen_t* screen;
xcb_window_t win;
xcb_intern_atom_reply_t* atomWMDeleteWindow;

void openWindow() {
	con = xcb_connect(NULL, NULL);
	screen = xcb_setup_roots_iterator(xcb_get_setup(con)).data;
	win = xcb_generate_id(con);
				
	xcb_create_window(con, XCB_COPY_FROM_PARENT, win, screen->root, 
		0, 0, 800, 600, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT, 
	screen->root_visual, 0, NULL);
	
	xcb_intern_atom_cookie_t cookie = 
		xcb_intern_atom(con, 1, 12, "WM_PROTOCOLS");
	xcb_intern_atom_reply_t* reply = 
		xcb_intern_atom_reply(con, cookie, 0);

	xcb_intern_atom_cookie_t cookie2 = 
		xcb_intern_atom(con, 0, 16, "WM_DELETE_WINDOW");
	atomWMDeleteWindow = xcb_intern_atom_reply(con, cookie2, 0);

	xcb_change_property(con, XCB_PROP_MODE_REPLACE,
		win, (*reply).atom, 4, 32, 1,
		&(*atomWMDeleteWindow).atom);

	xcb_change_property(con, XCB_PROP_MODE_REPLACE,
		win, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
		7, "Vulkan");

	free(reply);
																
	xcb_map_window(con, win);

	// Get Vulkan surface
	VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {
		VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR, 
		NULL, 0, con, win
	};

	vkCreateXcbSurfaceKHR(
		instance, &surfaceCreateInfo, NULL, &surface
	);

	// VkFormat colorFormat;
	// unsigned formatCount = 0;
	// vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GPUs[0], surface, &formatCount);

	// Create a swapchain
	VkSwapchainCreateInfoKHR swapCreateInfo = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, 
		NULL, 0, surface, 1, VK_FORMAT_R8G8B8_UNORM, 
		VK_COLORSPACE_SRGB_NONLINEAR_KHR, {800, 600}, 
		1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
		VK_SHARING_MODE_EXCLUSIVE, 0, NULL, 0, 0, 
		VK_PRESENT_MODE_IMMEDIATE_KHR, 
	};
	vkCreateSwapchainKHR(device, &swapCreateInfo, NULL, &swap);

	// Get actual images for the swapchain
	vkGetSwapchainImagesKHR(
		device, swap, &swapCount, swapImages
	);

	printf("Swapchain length: %d\n", swapCount);

	for (int i = 0; i < swapCount; i++) {
		swapViews[i] = (VkImageViewCreateInfo) {
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, 
			NULL, 0, swapImages[i], VK_IMAGE_VIEW_TYPE_2D, 
			VK_FORMAT_R8G8B8_UNORM, {
				VK_COMPONENT_SWIZZLE_R, 
				VK_COMPONENT_SWIZZLE_G, 
				VK_COMPONENT_SWIZZLE_B, 
				VK_COMPONENT_SWIZZLE_A
			}, {
				VK_IMAGE_ASPECT_COLOR_BIT, 
				0, 1, 0, 1
			}
		};

		VkImageView view;
		vkCreateImageView(device, &swapViews[i], NULL, &view);
	}
}

void prepRender() {
	vkAcquireNextImageKHR(
		device, swap, UINT64_MAX, VK_NULL_HANDLE, 
		VK_NULL_HANDLE, &nextImage
	);
}

void tickWindow() {
	VkPresentInfoKHR presentInfo = {
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, 
		NULL, 0, NULL, 1, &swap, &nextImage, NULL
	};
	vkQueuePresentKHR(queue, &presentInfo);

	xcb_flush(con);

	xcb_generic_event_t* event;
	while ((event = xcb_poll_for_event(con))) {
		// Close at first opportunity
		isClosing = true;
	}
}

void quitWindow() {
	xcb_destroy_window(con, win);

	xcb_disconnect(con);
}

