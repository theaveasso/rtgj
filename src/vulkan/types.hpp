#pragma once

#include <volk.h>

struct RendererContext {
	VkInstance       instance;
	VkPhysicalDevice physical_device;
	VkDevice         device;
	VkSwapchainKHR   swapchain;
};