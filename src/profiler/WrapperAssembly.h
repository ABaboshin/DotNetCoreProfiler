#pragma once

#include "types.h"
#include "util.h"

struct WrapperAssembly
{
	const WSTRING WrapperAssemblyPath;
	const WSTRING WrapperAssemblyName;

	WrapperAssembly(WSTRING WrapperAssemblyPath, WSTRING WrapperAssemblyName) :
		WrapperAssemblyName(WrapperAssemblyName),
		WrapperAssemblyPath(WrapperAssemblyPath) {}

	WrapperAssembly() :
		WrapperAssemblyName(""_W),
		WrapperAssemblyPath(""_W) {}

	friend bool operator==(const WrapperAssembly& lhs, const WrapperAssembly& rhs);
};

inline bool operator==(const WrapperAssembly& lhs, const WrapperAssembly& rhs)
{
	return lhs.WrapperAssemblyPath == rhs.WrapperAssemblyPath && lhs.WrapperAssemblyName == rhs.WrapperAssemblyName;
}