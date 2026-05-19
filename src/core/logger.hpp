#pragma once

#include <spdlog/spdlog.h>

bool logger_init();
void logger_shutdown();

#define RTGJ_LOG_TRACE(...)    SPDLOG_TRACE(__VA_ARGS__)
#define RTGJ_LOG_DEBUG(...)    SPDLOG_DEBUG(__VA_ARGS__)
#define RTGJ_LOG_INFO(...)     SPDLOG_INFO(__VA_ARGS__)
#define RTGJ_LOG_WARN(...)     SPDLOG_WARN(__VA_ARGS__)
#define RTGJ_LOG_ERROR(...)    SPDLOG_ERROR(__VA_ARGS__)
#define RTGJ_LOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

#ifdef NDEBUG
#define RTGJ_ASSERT(condition, message)
do {
	if (!(condition)) {
		throw std::runtime_error(message);
	}
} while (false)
#else
#define ASSERT(condition, message) assert((condition) && (message))
#endif

    // #define VK_CHECK(vk_func) \
// 	do {
    //     if (const VkResult check_result = (vk_func); check_result != VK_SUCCESS) {
    // 	const char* err = string_VkResult(check_result);
    // 	RTGJ_LOG_ERROR(...);
    // 	RTGJ_ASSERT(check_result == VK_SUCCESS, err);
    // }
    // }
    // while (0)