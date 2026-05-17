#pragma once

#include <string_view>

void platform_init(int width, int height, std::string_view title);
void platform_update_and_render();
void platform_shutdown();