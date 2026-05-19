#include "renderer.hpp"

#include <volk.h>

struct Context {
	VkInstance       instance;
	VkPhysicalDevice physical_device;
	VkDevice         device;
	VkSurfaceKHR     surface;

	Context();
	~Context();
};

static Context g_context = {};

bool renderer_init() {
	return true;
}

void renderer_shutdown() {
}