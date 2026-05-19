#include "core/logger.hpp"
#include "platform/platform.hpp"

int main() {
	if (!logger_init() || !platform_init(800, 600, "RTGJ")) {
		RTGJ_LOG_CRITICAL("Failed to init systems");
		return 1;
	}

	RTGJ_LOG_TRACE("Trace");
	RTGJ_LOG_DEBUG("Debug");
	RTGJ_LOG_WARN("Warn");
	RTGJ_LOG_INFO("Info");

	RTGJ_ASSERT(1 != 1, "Testing assertion");

	while (!platform_should_quit()) {
		platform_pump_messages();
	}

	platform_shutdown();
	logger_shutdown();
	return 0;
}