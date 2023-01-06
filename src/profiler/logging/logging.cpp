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
	static bool jsonFormat = false;

	static std::map<LogLevel, const char*> logLevelMap =
	{
		{LogLevel::VERBOSE, "VERBOSE"},
		{LogLevel::DEBUG, "DEBUG"},
		{LogLevel::INFO, "INFO"},
		{LogLevel::ERR, "ERROR"},
		{LogLevel::NONE, "NONE"}
	};

	static std::map<bool, const char*> logFormatMap =
	{
		{true, "JSON"},
		{false, "PLAIN"},
	};

	void init()
	{
		auto logLevelEnv = util::GetEnvironmentValue("PROFILER_LOG_LEVEL");
		auto logLevelMappingResult = std::find_if(
			logLevelMap.begin(),
			logLevelMap.end(),
			[logLevelEnv](const std::pair<LogLevel, const char*> &mo) {return mo.second == logLevelEnv; });

		minLogLevel = logLevelMappingResult != logLevelMap.end() ? logLevelMappingResult->first : LogLevel::INFO;

		auto logFormatEnv = util::GetEnvironmentValue("PROFILER_LOG_FORMAT");
		auto logFormatMappingResult = std::find_if(
			logFormatMap.begin(),
			logFormatMap.end(),
			[logFormatEnv](const std::pair<bool, const char*>& mo) {return mo.second == logFormatEnv; });

		jsonFormat = logFormatMappingResult != logFormatMap.end() ? logFormatMappingResult->first : true;
	}

	bool IsLogLevelEnabled(LogLevel logLevel)
	{
		return logLevel >= minLogLevel;
	}

	void log(LogLevel logLevel, const util::wstring& string)
	{
		if (jsonFormat)
		{
			nlohmann::json j;

			j["level"] = logLevelMap[logLevel];
			j["message"] = util::ToString(string);
			j["source"] = "profiler";

			std::cout << j.dump() << std::endl;
		}
		else
		{
			std::cout << "[profiler]" << "[" << logLevelMap[logLevel] << "]" << util::ToString(string) << std::endl;
		}
		
	}
}