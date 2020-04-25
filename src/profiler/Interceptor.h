#pragma once

#include <vector>
#include "util.h"

struct Interceptor {
	wstring AssemblyName;
	wstring TypeName;
	wstring MethodName;
	std::vector<BYTE> Signature;

	Interceptor(const wstring& assemblyName, const wstring& typeName, const wstring& methodName, std::vector<BYTE> signature) :
		AssemblyName(assemblyName),
		TypeName(typeName),
		MethodName(methodName),
		Signature(signature) {}

	Interceptor() :
		AssemblyName(""_W),
		TypeName(""_W),
		MethodName(""_W) {}

	Interceptor& operator=(const Interceptor& b)
	{
		if (this != &b)
		{
			AssemblyName = b.AssemblyName;
			TypeName = b.TypeName;
			MethodName = b.MethodName;
			Signature = b.Signature;
		}

		return *this;
	}
};
