#pragma once

#include <vector>
#include "util.h"

struct Interceptor {
	wstring AssemblyName;
	wstring TypeName;

	Interceptor(const wstring& assemblyName, const wstring& typeName) :
		AssemblyName(assemblyName),
		TypeName(typeName) {}

	Interceptor() :
		AssemblyName(""_W),
		TypeName(""_W) {}

	Interceptor& operator=(const Interceptor& b)
	{
		if (this != &b)
		{
			AssemblyName = b.AssemblyName;
			TypeName = b.TypeName;
		}

		return *this;
	}
};
