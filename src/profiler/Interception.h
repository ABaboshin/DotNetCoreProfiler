#pragma once

#include <fstream>
#include <vector>
#include <utility>
#include "nlohmann/json.hpp"
#include "types.h"
#include "util.h"

struct Interception {
	const WSTRING AssemblyName;
	const WSTRING TypeName;
	const WSTRING MethodName;
	const bool isCounter;
	const WSTRING WrapperAssemblyPath;
	const WSTRING WrapperAssemblyName;
	const WSTRING WrapperTypeName;
	const WSTRING WrapperMethodName;

	Interception(WSTRING AssemblyName, WSTRING TypeName, WSTRING MethodName, bool isCounter, WSTRING WrapperAssemblyPath, WSTRING WrapperAssemblyName, WSTRING WrapperTypeName, WSTRING WrapperMethodName) :
		AssemblyName(AssemblyName),
		TypeName(TypeName),
		MethodName(MethodName),
		isCounter(isCounter),
		WrapperAssemblyName(WrapperAssemblyName),
		WrapperTypeName(WrapperTypeName),
		WrapperMethodName(WrapperMethodName),
		WrapperAssemblyPath(WrapperAssemblyPath) {}

	Interception() :
		AssemblyName(""_W),
		TypeName(""_W),
		MethodName(""_W),
		isCounter(false),
		WrapperAssemblyName(""_W),
		WrapperTypeName(""_W),
		WrapperMethodName(""_W),
		WrapperAssemblyPath(""_W) {}
};

std::vector<Interception> LoadFromFile(const WSTRING& path);
std::vector<Interception> LoadFromStream(std::ifstream& stream);
std::pair<Interception, bool> LoadFromJson(const nlohmann::json::value_type& src);

WSTRING ToString(const Interception& interception);