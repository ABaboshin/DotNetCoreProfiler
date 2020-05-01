#pragma once

#include "util/util.h"
#include"Interceptor.h"
#include "TargetMethod.h"

struct Interception {
	wstring callerAssemblyName;
	TargetMethod Target{};
	Interceptor Interceptor{};

	Interception(const wstring& callerAssemblyName, const TargetMethod& targetMethod, const ::Interceptor& interceptor) :
		callerAssemblyName(callerAssemblyName),
		Target(targetMethod),
		Interceptor(interceptor) {}

	Interception() :
		callerAssemblyName(""_W) {}
};
