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
#include <string.h>
#include "window.h"

// VULKAN
#include "util/vulkan.h"

bool isClosing = false;
VkSurfaceKHR surface;
unsigned nextImage = 0;

#ifdef XCB_WINDOWING
xcb_connection_t* con;
xcb_screen_t* screen;
xcb_window_t win;
xcb_intern_atom_reply_t* atomWMDeleteWindow;
#elif defined(X11_WINDOWING)
Display* d;
Window w;
XEvent e;
#endif

void openWindow() {
#ifdef X11_WINDOWING
	d = XOpenDisplay(NULL);

	int s = DefaultScreen(d);

	w = XCreateSimpleWindow(d, RootWindow(d, s), 
		0, 0, 800, 600, 1, BlackPixel(d, s), WhitePixel(d, s)
	);

	XSelectInput(d, w, ExposureMask | KeyPressMask);
	XMapWindow(d, w);

#elif defined(XCB_WINDOWING)
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

	check(vkCreateXcbSurfaceKHR(
		instance, &surfaceCreateInfo, NULL, &surface
	));
#endif

	unsigned numFormats = 16;
	VkSurfaceFormatKHR formats[numFormats];
	memset(formats, 0, sizeof(formats));
	for (int i = 0; i < numFormats; i++)
		formats[i].format = VK_FORMAT_R8G8B8_UINT;

	// check(vkGetPhysicalDeviceSurfaceFormatsKHR(
	// 	GPUs[0], surface, &numFormats, formats
	// ));

	// VkFormat colorFormat;
	// unsigned formatCount = 0;
	// vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GPUs[0], surface, &formatCount);

	// Create a swapchain
	VkSwapchainCreateInfoKHR swapCreateInfo = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, NULL, 0,
		surface, 1, formats[0].format, 
		formats[0].colorSpace, {800, 600}, 
		1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT, 
		VK_SHARING_MODE_EXCLUSIVE, 0, NULL, 
		VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, 
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, 
		VK_PRESENT_MODE_IMMEDIATE_KHR, true, VK_NULL_HANDLE
	};
	check(vkCreateSwapchainKHR(device, &swapCreateInfo, NULL, &swap));

	// Get actual images for the swapchain
	check(vkGetSwapchainImagesKHR(
		device, swap, &swapCount, swapImages
	));

	printf("Swapchain length: %d\n", swapCount);

	for (int i = 0; i < swapCount; i++) {
		swapViews[i] = (VkImageViewCreateInfo) {
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, 
			NULL, 0, swapImages[i], VK_IMAGE_VIEW_TYPE_2D, 
			formats[0].format, {
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
		check(vkCreateImageView(device, &swapViews[i], NULL, &view));
	}
}

void prepRender() {
	check(vkAcquireNextImageKHR(
		device, swap, 1000000000000, VK_NULL_HANDLE, 
		VK_NULL_HANDLE, &nextImage
	));
}

void tickWindow() {
	VkPresentInfoKHR presentInfo = {
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, 
		NULL, 0, NULL, 1, &swap, &nextImage, NULL
	};
	check(vkQueuePresentKHR(queue, &presentInfo));

#ifdef X11_WINDOWING
	XNextEvent(d, &e);


#elif XCB_WINDOWING
	xcb_flush(con);

	xcb_generic_event_t* event;
	while ((event = xcb_poll_for_event(con))) {
		// Close at first opportunity
		isClosing = true;
	}
#endif
}

void quitWindow() {
#ifdef XCB_WINDOWING
	xcb_destroy_window(con, win);

	xcb_disconnect(con);
#elif defined(X11_WINDOWING)
#endif
}

