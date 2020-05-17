#pragma once

#include "util/types.h"

using namespace util;

namespace configuration
{
	struct AssemblyInitializer
	{
		wstring AssemblyName;
		wstring AssemblyPath;
		wstring InitializerType;

		AssemblyInitializer(wstring assemblyName, wstring assemblyPath, wstring initializerType) : AssemblyName(assemblyName), AssemblyPath(assemblyPath), InitializerType(initializerType){}

		AssemblyInitializer() {}
	};
}