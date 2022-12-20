#pragma once

#include <unordered_set>
#include "TypeInfo.h"
#include "TargetMethod.h"
#include "util/util.h"

using namespace util;

namespace configuration
{
	struct StrictInterception {
		//std::unordered_set<wstring> IgnoreCallerAssemblies{};
		TargetMethod Target{};
		TypeInfo Interceptor{};

		StrictInterception(
			//const std::unordered_set<wstring>& ignoreCallerAssemblies,
			const TargetMethod& targetMethod, const ::configuration::TypeInfo& interceptor) :
			//IgnoreCallerAssemblies(ignoreCallerAssemblies),
			Target(targetMethod),
			Interceptor(interceptor) {}

		StrictInterception() {}
	};
}
