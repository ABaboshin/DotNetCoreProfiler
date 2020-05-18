#pragma once

#include <vector>
#include "util/util.h"

namespace configuration
{
	using namespace util;

	struct Interceptor {
		wstring AssemblyName;
		wstring TypeName;

		Interceptor(const wstring& assemblyName, const wstring& typeName) :
			AssemblyName(assemblyName),
			TypeName(typeName) {}

		Interceptor() :
			AssemblyName(""_W),
			TypeName(""_W) {}
	};
}
