#include "platform/platform.hpp"

int main() {
	if (!platform_init(800, 600, "RTGJ")) {
		return 1;
	}

	while (!platform_should_quit()) {
		platform_pump_messages();
	}

	platform_shutdown();
	return 0;
}