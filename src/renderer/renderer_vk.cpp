#include "renderer.hpp"

#include "core/logger.hpp"
#include "vulkan/types.hpp"
#include "vulkan/utils.hpp"

#include <vector>

static RendererContext g_context = {};

static std::vector<const char*> g_layers     = {};

constexpr const char* validation_layer_name = "VK_LAYER_KHRONOS_validation";

bool renderer_init() {
	VK_CHECK(volkInitialize());

	auto app_info               = VkApplicationInfo();
	app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext              = nullptr;
	app_info.pApplicationName   = "RTGJ";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName        = "RTGJEngine";
	app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion         = VK_API_VERSION_1_4;

	auto create_info              = VkInstanceCreateInfo();
	create_info.sType             = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pNext             = nullptr;
	create_info.flags             = 0;
	create_info.pApplicationInfo  = &app_info;
	create_info.enabledLayerCount = 0;

	std::vector<const char*> extensions = { VK_KHR_SURFACE_EXTENSION_NAME };

#if defined(_WIN32)
	extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

#if defined(RTGJ_DEBUG)

#endif


	return true;
}

void renderer_shutdown() {
}