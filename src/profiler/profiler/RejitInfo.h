#pragma once

#include <memory>
#include "cor.h"
#include "corprof.h"
#include "configuration/Configuration.h"
#include "info/FunctionInfo.h"

struct RejitInfo
{
	ModuleID ModuleId;
	mdMethodDef MethodId;
	info::FunctionInfo Info;
	std::vector<configuration::StrictInterception> Interceptors;
	bool Trace;
	std::vector<util::wstring> Parameters{};
	std::vector<util::wstring> Variables{};
	util::wstring TraceName;
	std::vector<int> Offsets;

	RejitInfo() {}
	RejitInfo(ModuleID m, mdMethodDef f, info::FunctionInfo i, std::vector<configuration::StrictInterception> interceptors, bool trace, const std::vector<util::wstring>& parameters, const util::wstring& traceName, const std::vector<int> offsets) : ModuleId(m), MethodId(f), Info(i), Interceptors(interceptors), Trace(trace), Parameters(parameters), TraceName(traceName), Offsets(offsets) {}
};
