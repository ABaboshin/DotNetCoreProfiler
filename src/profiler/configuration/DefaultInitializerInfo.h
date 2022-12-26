#pragma once

#include "util/util.h"

namespace configuration
{
	using namespace util;

	struct DefaultInitializerInfo {
        wstring AssemblyName;
        wstring AssemblyPath;
        wstring TypeName;
        wstring MethodName;
        bool NotFound = true;

        DefaultInitializerInfo(const wstring& assemblyPath, const wstring& typeName, const wstring& methodName, const wstring& assemblyName) : AssemblyPath(assemblyPath),
            TypeName(typeName),
            MethodName(methodName),
            AssemblyName(assemblyName) {}

        DefaultInitializerInfo() : AssemblyPath(""_W),
            TypeName(""_W),
            MethodName(""_W),
            AssemblyName(""_W) {}
	};
}
