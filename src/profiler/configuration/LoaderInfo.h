#pragma once

#include <vector>
#include "util/util.h"

namespace configuration
{
	using namespace util;

	struct LoaderInfo {
		wstring AssemblyPath;
		wstring TypeName;

		LoaderInfo(const wstring& assemblyPath, const wstring& typeName) :
			AssemblyPath(assemblyPath),
			TypeName(typeName) {}

		LoaderInfo() :
			AssemblyPath(""_W),
			TypeName(""_W) {}
	};
}
