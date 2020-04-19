#pragma once

#include <fstream>
#include <vector>
#include <utility>
#include "nlohmann/json.hpp"
#include "types.h"
#include "util.h"

struct Interception {
	const WSTRING CallerAssemblyName;
	const WSTRING TargetAssemblyName;
	const WSTRING TargetTypeName;
	const WSTRING TargetMethodName;
	const WSTRING WrapperAssemblyPath;
	const WSTRING WrapperAssemblyName;
	const WSTRING WrapperTypeName;
	const WSTRING WrapperMethodName;
	const std::vector<BYTE> signature;
	const int TargetMethodParametersCount;

	Interception(WSTRING CallerAssemblyName, WSTRING AssemblyName, WSTRING TypeName, WSTRING MethodName, WSTRING WrapperAssemblyPath, WSTRING WrapperAssemblyName, WSTRING WrapperTypeName, WSTRING WrapperMethodName, std::vector<BYTE> signature, int TargetMethodParametersCount) :
		CallerAssemblyName(CallerAssemblyName),
		TargetAssemblyName(AssemblyName),
		TargetTypeName(TypeName),
		TargetMethodName(MethodName),
		WrapperAssemblyName(WrapperAssemblyName),
		WrapperTypeName(WrapperTypeName),
		WrapperMethodName(WrapperMethodName),
		WrapperAssemblyPath(WrapperAssemblyPath),
		TargetMethodParametersCount(TargetMethodParametersCount),
		signature(signature) {}

	Interception() :
		CallerAssemblyName(""_W),
		TargetAssemblyName(""_W),
		TargetTypeName(""_W),
		TargetMethodName(""_W),
		WrapperAssemblyName(""_W),
		WrapperTypeName(""_W),
		WrapperMethodName(""_W),
		WrapperAssemblyPath(""_W),
		TargetMethodParametersCount(0) {}
};
