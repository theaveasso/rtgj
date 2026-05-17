#include "platform.hpp"

#ifdef _WIN32
#include <windows.h>

static HWND window_handle = nullptr;
static HINSTANCE instance_handle = nullptr;

void platform_init(int width, int height, std::string_view title) {
}

void platform_update_and_render() {

}

void platform_shutdown() {

}
#endif