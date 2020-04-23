#pragma once

#include <fstream>
#include <vector>
#include <utility>
#include "util.h"

struct Interception {
	wstring callerAssemblyName;
	wstring targetAssemblyName;
	wstring targetTypeName;
	wstring targetMethodName;
	wstring wrapperAssemblyPath;
	wstring wrapperAssemblyName;
	wstring wrapperTypeName;
	wstring wrapperMethodName;
	std::vector<BYTE> signature;
	const int targetMethodParametersCount;

	Interception(const wstring& callerAssemblyName, const wstring& assemblyName, const wstring& typeName, const wstring& methodName, const wstring& wrapperAssemblyPath, const wstring& wrapperAssemblyName, const wstring& wrapperTypeName, const wstring& wrapperMethodName, std::vector<BYTE> signature, int targetMethodParametersCount) :
		callerAssemblyName(callerAssemblyName),
		targetAssemblyName(assemblyName),
		targetTypeName(typeName),
		targetMethodName(methodName),
		wrapperAssemblyName(wrapperAssemblyName),
		wrapperTypeName(wrapperTypeName),
		wrapperMethodName(wrapperMethodName),
		wrapperAssemblyPath(wrapperAssemblyPath),
		targetMethodParametersCount(targetMethodParametersCount),
		signature(signature) {}

	Interception() :
		callerAssemblyName(""_W),
		targetAssemblyName(""_W),
		targetTypeName(""_W),
		targetMethodName(""_W),
		wrapperAssemblyName(""_W),
		wrapperTypeName(""_W),
		wrapperMethodName(""_W),
		wrapperAssemblyPath(""_W),
		targetMethodParametersCount(0) {}
};
