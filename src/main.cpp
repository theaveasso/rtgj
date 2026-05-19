#include "core/logger.hpp"
#include "platform/platform.hpp"
#include "renderer/renderer.hpp"

int main() {
	if (
		!logger_init() || 
		!platform_init(800, 600, "RTGJ") || 
		!renderer_init()
	) {
		RTGJ_LOG_CRITICAL("Failed to init systems");
		return 1;
	}

	while (!platform_should_quit()) {
		platform_pump_messages();
	}

	platform_shutdown();
	logger_shutdown();
	return 0;
}