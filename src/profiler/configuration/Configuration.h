#pragma once

#include <vector>
#include <unordered_set>
#include <memory>
#include "TypeInfo.h"
#include "StrictInterception.h"
#include "InterceptorMethodInfo.h"
#include "TraceMethodInfo.h"

namespace configuration
{
	struct Configuration
	{
		std::vector<StrictInterception> StrictInterceptions{};
		std::unordered_set<wstring> SkipAssemblies;
		TypeInfo Loader;
		InterceptorMethodInfo DefaultInitializer;
		std::shared_ptr<InterceptorMethodInfo> ExceptionLogger;
		std::shared_ptr<InterceptorMethodInfo> TracingBeginMethod;
		std::shared_ptr<InterceptorMethodInfo> TracingEndMethod;
		std::shared_ptr<InterceptorMethodInfo> TracingAddParameterMethod;
		std::vector<TraceMethodInfo> Traces{};

		Configuration(std::vector<StrictInterception> strictInterceptions,
			std::unordered_set<wstring>& skipAssemblies,
			TypeInfo loader,
			InterceptorMethodInfo defaultInitializer,
			std::shared_ptr<InterceptorMethodInfo> exceptionLogger,
			std::shared_ptr<InterceptorMethodInfo> tracingBeginMethod,
			std::shared_ptr<InterceptorMethodInfo> tracingEndMethod,
			std::shared_ptr<InterceptorMethodInfo> tracingAddParameterMethod,
			std::vector<TraceMethodInfo> traces
		) :
			StrictInterceptions(strictInterceptions),
			SkipAssemblies(skipAssemblies),
			Loader(loader),
			DefaultInitializer(defaultInitializer),
			ExceptionLogger(exceptionLogger),
			TracingBeginMethod(tracingBeginMethod),
			TracingEndMethod(tracingEndMethod),
			TracingAddParameterMethod(tracingAddParameterMethod),
			Traces(traces)
		{}

		Configuration() {}

		static Configuration LoadFromStream(std::istream& stream);
		static Configuration LoadConfiguration(const std::string& path);
	};
}
