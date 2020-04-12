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

	Interception(WSTRING CallerAssemblyName, WSTRING AssemblyName, WSTRING TypeName, WSTRING MethodName, WSTRING WrapperAssemblyPath, WSTRING WrapperAssemblyName, WSTRING WrapperTypeName, WSTRING WrapperMethodName, std::vector<BYTE> signature) :
		CallerAssemblyName(CallerAssemblyName),
		TargetAssemblyName(AssemblyName),
		TargetTypeName(TypeName),
		TargetMethodName(MethodName),
		WrapperAssemblyName(WrapperAssemblyName),
		WrapperTypeName(WrapperTypeName),
		WrapperMethodName(WrapperMethodName),
		WrapperAssemblyPath(WrapperAssemblyPath),
		signature(signature) {}

	Interception() :
		CallerAssemblyName(""_W),
		TargetAssemblyName(""_W),
		TargetTypeName(""_W),
		TargetMethodName(""_W),
		WrapperAssemblyName(""_W),
		WrapperTypeName(""_W),
		WrapperMethodName(""_W),
		WrapperAssemblyPath(""_W) {}
};

std::vector<Interception> LoadFromFile(const WSTRING& path);
std::vector<Interception> LoadFromStream(std::ifstream& stream);
std::pair<Interception, bool> LoadFromJson(const nlohmann::json::value_type& src);

WSTRING ToString(const Interception& interception);