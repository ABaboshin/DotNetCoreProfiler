#pragma once

#include "fmt/fmt.h"
#include "util/types.h"

namespace logging
{
	enum class LogLevel
	{
		DEBUG = 0,
		INFO = 1
	};

	void init();

	void log(LogLevel logLevel, const util::wstring& string);

	bool IsLogLevelEnabled(LogLevel logLevel);

	template <typename S, typename... Args>
	inline void log(LogLevel logLevel, const S& format_str, Args&&... args) {
		if (IsLogLevelEnabled(logLevel))
		{
			log(logLevel, fmt::format(format_str, args...));
		}
	}
}