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
		bool IsComposed = false;
		wstring Key;

		Interception(const wstring& callerAssemblyName, const TargetMethod& targetMethod, const ::configuration::Interceptor& interceptor, bool isComposed, wstring key) :
			CallerAssemblyName(callerAssemblyName),
			Target(targetMethod),
			Interceptor(interceptor),
			IsComposed(isComposed),
			Key(key) {}

		Interception() :
			CallerAssemblyName(""_W) {}
	};
}
