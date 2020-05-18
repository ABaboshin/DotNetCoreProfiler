#pragma once

#include "util/util.h"

namespace configuration
{
	using namespace util;

	struct TargetMethod {
		wstring AssemblyName;
		wstring TypeName;
		wstring MethodName;
		int MethodParametersCount;

		TargetMethod(const wstring& assemblyName, const wstring& typeName, const wstring& methodName, int methodParametersCount) :
			AssemblyName(assemblyName),
			TypeName(typeName),
			MethodName(methodName),
			MethodParametersCount(methodParametersCount) {}

		TargetMethod() :
			AssemblyName(""_W),
			TypeName(""_W),
			MethodName(""_W),
			MethodParametersCount(0) {}
	};
}