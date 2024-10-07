#pragma once

#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#define LOGGER_FORMAT "[%^%l%$] %v"
#define PROJECT_NAME "ToyEngine"

#define LOGI(...) spdlog::info("{}", fmt::format(__VA_ARGS__));
#define LOGW(...) spdlog::warn(__VA_ARGS__);
#define LOGE(...) spdlog::error("{}", fmt::format(__VA_ARGS__));
#define LOGD(...) spdlog::debug("{}", fmt::format(__VA_ARGS__));

namespace logging 
{
	static void init() 
	{
		auto env_val = spdlog::details::os::getenv("TE_LOG_LEVEL");
		if (!env_val.empty())
		{
			spdlog::cfg::helpers::load_levels(env_val);
		}

		auto logger = spdlog::stdout_color_mt("ToyLogger");

		logger->set_pattern(LOGGER_FORMAT);
		logger->set_level(spdlog::level::trace);
		spdlog::set_default_logger(logger);
	}
}

