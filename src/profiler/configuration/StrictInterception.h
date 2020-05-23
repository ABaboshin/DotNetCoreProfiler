#pragma once

#include"Interceptor.h"
#include "TargetMethod.h"
#include "util/util.h"

using namespace util;

namespace configuration
{
	struct StrictInterception {
		std::vector<wstring> IgnoreCallerAssemblies{};
		TargetMethod Target{};
		Interceptor Interceptor{};

		StrictInterception(const std::vector<wstring>& ignoreCallerAssemblies, const TargetMethod& targetMethod, const ::configuration::Interceptor& interceptor) :
			IgnoreCallerAssemblies(ignoreCallerAssemblies),
			Target(targetMethod),
			Interceptor(interceptor) {}

		StrictInterception() {}
	};
}
