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

#if defined(RTGJ_DEBUG)
#if defined(_MSC_VER)
#define RTGJ_ASSERT(condition, message)                                       \
	do {                                                                      \
		if (!(condition)) {                                                   \
			RTGJ_LOG_ERROR("assertion failed: ({}) {}", #condition, message); \
			__debugbreak();                                                   \
		}                                                                     \
	} while (false)
#elif defined(__GNUC__) || defined(__clang)
#define RTGJ_ASSERT(condition, message)                                       \
	do {                                                                      \
		if (!(condition)) {                                                   \
			RTGJ_LOG_ERROR("assertion failed: ({}) {}", #condition, message); \
			__builtin_debugtrap();                                            \
		}                                                                     \
	} while (false)
#endif
#else
#define RTGJ_ASSERT(condition, message) \
	do {
(void)sizeof(condition);
(void)sizeof(message);
}
while (false)
#endif