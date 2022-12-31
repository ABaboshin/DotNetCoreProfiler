#pragma once

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

  RejitInfo() {}
  RejitInfo(ModuleID m, mdMethodDef f, info::FunctionInfo i, std::vector<configuration::StrictInterception> interceptors) : ModuleId(m), MethodId(f), Info(i), Interceptors(interceptors) {}
};
