#include "platform.hpp"

#ifdef _WIN32
#include <windows.h>

static HWND 			g_window 			= nullptr;
static HINSTANCE 	g_instance 		= nullptr;
static bool 			g_should_quit	= false;

static const char* WINDOW_CLASS_NAME = "RTGJWindowClass";

static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

bool platform_init(int width, int height, std::string title) {
	g_instance = GetModuleHandle(nullptr);

	WNDCLASSEX wc = {};
	wc.cbSize 				= sizeof(wc);
	wc.style 					= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc 		= window_proc;
	wc.hInstance 			= g_instance;
	wc.hIcon 					= LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor 				= LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground 	= nullptr;
	wc.lpszMenuName 	= nullptr;
	wc.lpszClassName 	= WINDOW_CLASS_NAME;
	
	if (!RegisterClassEx(&wc)) {
		return false;
	}

	DWORD style = WS_OVERLAPPEDWINDOW;
	RECT rect = { 0, 0, width, height };
	AdjustWindowRect(&rect, style, FALSE);
	int outer_w = rect.right - rect.left;
	int outer_h = rect.bottom - rect.top;

	g_window = CreateWindowEx(
		0,
		WINDOW_CLASS_NAME,
		title.c_str(),
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		outer_w,
		outer_h,
		nullptr,
		nullptr,
		g_instance,
		nullptr
	);
	if (!g_window) {
		return false;
	}

	ShowWindow(g_window, SW_SHOW);
	UpdateWindow(g_window);
	return true;
}
void platform_shutdown() {
	if (g_window) {
		DestroyWindow(g_window);
		g_window = nullptr;
	}
	UnregisterClass(WINDOW_CLASS_NAME, g_instance);
}

void platform_pump_messages() {
	MSG msg;
	while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			g_should_quit = true;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

bool platform_should_quit() { return g_should_quit; }

void* platform_get_instance_handle() { return g_instance; }

void* platform_get_window_handle() { return g_window; }

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch(msg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}

#endif