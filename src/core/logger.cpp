#include "logger.hpp"

#include <memory>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

static std::shared_ptr<spdlog::logger> g_logger;

constexpr int logger_max_file_size = 1048576 * 5;
constexpr int logger_max_files     = 3;

bool logger_init() {
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto rotate_sink  = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/rtgj.log", logger_max_file_size, logger_max_files);

	g_logger = std::make_shared<spdlog::logger>("rtgj_logger", spdlog::sinks_init_list{console_sink, rotate_sink});
#ifndef NDEBUG
	g_logger->set_level(spdlog::level::trace);
#else
	g_logger->set_level(spdlog::level::info);
#endif
	g_logger->set_pattern("[%H:%M:%S %z] [%^%L%$] [thread %t] %v");
	spdlog::flush_on(spdlog::level::warn);
	spdlog::set_default_logger(g_logger);

	return true;
}

void logger_shutdown() { spdlog::shutdown(); }