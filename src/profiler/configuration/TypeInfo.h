#pragma once

#include <vector>
#include "util/util.h"

namespace configuration
{
	using namespace util;

	struct TypeInfo {
		wstring AssemblyName;
		wstring TypeName;
		wstring AssemblyPath;

		TypeInfo(const wstring& assemblyName, const wstring& typeName, const wstring& assemblyPath) :
			AssemblyName(assemblyName),
			TypeName(typeName),
			AssemblyPath(assemblyPath) {}

		TypeInfo() :
			AssemblyName(""_W),
			TypeName(""_W) {}
	};
}
