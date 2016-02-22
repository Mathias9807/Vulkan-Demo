#ifndef VULKAN_H
#define VULKAN_H


#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>

extern VkInstance		instance;
extern VkDevice			device;

extern VkQueue			queue;
extern VkCommandPool	comPool;
extern VkCommandBuffer	comBuffer;

extern VkSwapchainKHR	swap;
extern VkImage			swapImages[];
extern VkImageViewCreateInfo	swapViews[];
extern unsigned			nextImage;
extern unsigned			swapCount;

void createInstance(char* name);
void getDevice();
void createCommandPool();
void createCommandBuffer();
void beginCommands();
void endCommands();
void submitCommandBuffer();
void destroyInstance();
void check(VkResult r);


#endif // VULKAN_H
