#pragma once
#include "util/util.h"

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

	TargetMethod& operator=(const TargetMethod& b)
	{
		if (this != &b)
		{
			AssemblyName = b.AssemblyName;
			TypeName = b.TypeName;
			MethodName = b.MethodName;
			MethodParametersCount = b.MethodParametersCount;
		}

		return *this;
	}
};
