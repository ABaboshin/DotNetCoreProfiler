#pragma once

#include <vector>
#include "util/util.h"

namespace configuration
{
	using namespace util;

	struct BaseClass {
		wstring AssemblyName;
		wstring TypeName;

		BaseClass(const wstring& assemblyName, const wstring& typeName) :
			AssemblyName(assemblyName),
			TypeName(typeName) {}

		BaseClass() :
			AssemblyName(""_W),
			TypeName(""_W) {}
	};
}
