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
  std::shared_ptr<configuration::TraceMethodInfo> TraceMethodInfo{};

  RejitInfo() {}
  RejitInfo(ModuleID m, mdMethodDef f, info::FunctionInfo i, std::vector<configuration::StrictInterception> interceptors, std::shared_ptr<configuration::TraceMethodInfo> traceMethodInfo) : ModuleId(m), MethodId(f), Info(i), Interceptors(interceptors), TraceMethodInfo(traceMethodInfo) {}
};
