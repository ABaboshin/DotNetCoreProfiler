#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"
#include "logging/JsonFormatter.h"
#include "util.h"

namespace logging
{
	void init()
	{
		spdlog::cfg::load_env_levels();
		spdlog::set_formatter(spdlog::details::make_unique<logging::JsonFormatter>());
	}
}