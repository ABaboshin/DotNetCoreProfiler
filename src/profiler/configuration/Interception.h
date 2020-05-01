#pragma once

#include"Interceptor.h"
#include "TargetMethod.h"
#include "util/util.h"

namespace configuration
{
	struct Interception {
		wstring callerAssemblyName;
		TargetMethod Target{};
		Interceptor Interceptor{};

		Interception(const wstring& callerAssemblyName, const TargetMethod& targetMethod, const ::configuration::Interceptor& interceptor) :
			callerAssemblyName(callerAssemblyName),
			Target(targetMethod),
			Interceptor(interceptor) {}

		Interception() :
			callerAssemblyName(""_W) {}
	};
}
