#include "platform.hpp"

int main() {
	platform_init(800, 600, "RTGJ");

	while (!platform_should_quit()) {
		platform_pump_messages();
	}

	platform_shutdown();
	return 0;
}