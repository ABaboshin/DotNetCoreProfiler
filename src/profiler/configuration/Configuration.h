#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "TypeInfo.h"
#include "StrictInterception.h"
#include "InterceptorMethodInfo.h"
#include "TraceMethodInfo.h"
#include "InstrumentationConfiguration.h"

namespace configuration
{
	//struct TargetMethodHash
	//{
	//	template <class T1, class T2>
	//	std::size_t operator () (std::pair<T1, T2> const& v) const
	//	{
	//		return std::hash<T1>()(v.size());
	//	}
	//};

	struct Configuration
	{
		std::unordered_set<wstring> SkipAssemblies;
		TypeInfo Loader;
		InterceptorMethodInfo DefaultInitializer;
		std::shared_ptr<InterceptorMethodInfo> ExceptionLogger;
		std::shared_ptr<InterceptorMethodInfo> TracingBeginMethod;
		std::shared_ptr<InterceptorMethodInfo> TracingEndMethod;
		std::shared_ptr<InterceptorMethodInfo> TracingAddParameterMethod;
		std::shared_ptr<InterceptorMethodInfo> DebuggerBeginMethod;
		std::shared_ptr<InterceptorMethodInfo> DebuggerEndMethod;
		std::shared_ptr<InterceptorMethodInfo> DebuggerAddParameterMethod;
		std::unordered_map<TargetMethod, InstrumentationConfiguration> Instrumentations;

		Configuration(
			std::unordered_set<wstring>& skipAssemblies,
			TypeInfo loader,
			InterceptorMethodInfo defaultInitializer,
			std::shared_ptr<InterceptorMethodInfo> exceptionLogger,
			std::shared_ptr<InterceptorMethodInfo> tracingBeginMethod,
			std::shared_ptr<InterceptorMethodInfo> tracingEndMethod,
			std::shared_ptr<InterceptorMethodInfo> tracingAddParameterMethod,
			std::shared_ptr<InterceptorMethodInfo> debuggerBeginMethod,
			std::shared_ptr<InterceptorMethodInfo> debuggerEndMethod,
			std::shared_ptr<InterceptorMethodInfo> debuggerAddParameterMethod,
			std::unordered_map<TargetMethod, InstrumentationConfiguration> instrumentations
		) :
			SkipAssemblies(skipAssemblies),
			Loader(loader),
			DefaultInitializer(defaultInitializer),
			ExceptionLogger(exceptionLogger),
			TracingBeginMethod(tracingBeginMethod),
			TracingEndMethod(tracingEndMethod),
			TracingAddParameterMethod(tracingAddParameterMethod),
			DebuggerBeginMethod(debuggerBeginMethod),
			DebuggerEndMethod(debuggerEndMethod),
			DebuggerAddParameterMethod(debuggerAddParameterMethod),
			Instrumentations(instrumentations)
		{}

		Configuration() {}

		static Configuration LoadFromStream(std::istream& stream);
		static Configuration LoadConfiguration(const std::string& path);
	};
}
