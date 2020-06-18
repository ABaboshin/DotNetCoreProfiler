#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include "logging.h"
#include "util/util.h"
#include "logging/logging.h"

namespace logging
{
	using namespace util;
	static LogLevel minLogLevel;

	static std::map<LogLevel, const char*> logLevelMap =
	{
		{LogLevel::DEBUG, "DEBUG"},
		{LogLevel::INFO, "INFO"}
	};

	void init()
	{
		auto logLevelEnv = util::GetEnvironmentValue("PROFILER_LOG_LEVEL");
		auto result = std::find_if(
			logLevelMap.begin(),
			logLevelMap.end(),
			[logLevelEnv](const std::pair<LogLevel, const char*> &mo) {return mo.second == logLevelEnv; });

		minLogLevel = result != logLevelMap.end() ? result->first : LogLevel::INFO;
	}

	bool IsLogLevelEnabled(LogLevel logLevel)
	{
		return logLevel >= minLogLevel;
	}

	void log(LogLevel logLevel, const util::wstring& string)
	{
		nlohmann::json j;

		j["level"] = logLevelMap[logLevel];
		j["message"] = util::ToString(string);
		j["source"] = "profiler";

		std::cout << j.dump() << std::endl;
	}
}