#pragma once

#include"Interceptor.h"
#include "TargetMethod.h"
#include "util/util.h"

using namespace util;

namespace configuration
{
	struct StrictInterception {
		wstring CallerAssemblyName;
		TargetMethod Target{};
		Interceptor Interceptor{};

		StrictInterception(const wstring& callerAssemblyName, const TargetMethod& targetMethod, const ::configuration::Interceptor& interceptor) :
			CallerAssemblyName(callerAssemblyName),
			Target(targetMethod),
			Interceptor(interceptor) {}

		StrictInterception() :
			CallerAssemblyName(""_W) {}
	};
}
