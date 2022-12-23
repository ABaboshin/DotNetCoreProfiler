#pragma once

#include <vector>
#include "util/util.h"

namespace configuration
{
	using namespace util;

	struct DefaultInitializerInfo {
		wstring AssemblyPath;
		wstring TypeName;

		DefaultInitializerInfo(const wstring& assemblyPath, const wstring& typeName) :
			AssemblyPath(assemblyPath),
			TypeName(typeName) {}

		DefaultInitializerInfo() :
			AssemblyPath(""_W),
			TypeName(""_W) {}
	};
}
