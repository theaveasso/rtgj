#pragma once

#include <string>

bool platform_init(int width, int height, std::string title);
void platform_shutdown();
void platform_pump_messages();

bool platform_should_quit();

void* platform_get_instance_handle(void);
void* platform_get_window_handle(void);