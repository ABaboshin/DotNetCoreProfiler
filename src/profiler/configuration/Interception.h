#pragma once

#include"Interceptor.h"
#include "TargetMethod.h"
#include "util/util.h"

using namespace util;

namespace configuration
{
	struct Interception {
		wstring CallerAssemblyName;
		TargetMethod Target{};
		Interceptor Interceptor{};

		Interception(const wstring& callerAssemblyName, const TargetMethod& targetMethod, const ::configuration::Interceptor& interceptor) :
			CallerAssemblyName(callerAssemblyName),
			Target(targetMethod),
			Interceptor(interceptor) {}

		Interception() :
			CallerAssemblyName(""_W) {}
	};
}
