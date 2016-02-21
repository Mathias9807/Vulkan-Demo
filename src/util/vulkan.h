#ifndef VULKAN_H
#define VULKAN_H


#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>

extern VkInstance instance;

void createInstance(char* name);
void getDevice();
void destroyInstance();


#endif // VULKAN_H
