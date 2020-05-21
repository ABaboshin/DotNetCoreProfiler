#pragma once

#include <vector>
#include "util/util.h"

namespace configuration
{
	using namespace util;

	struct TypeInfo {
		wstring AssemblyName;
		wstring TypeName;

		TypeInfo(const wstring& assemblyName, const wstring& typeName) :
			AssemblyName(assemblyName),
			TypeName(typeName) {}

		TypeInfo() :
			AssemblyName(""_W),
			TypeName(""_W) {}
	};
}
