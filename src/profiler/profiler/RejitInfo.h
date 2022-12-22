#pragma once

#include "cor.h"
#include "corprof.h"
#include "configuration/Configuration.h"
#include "info/FunctionInfo.h"

struct RejitInfo
{
  ModuleID moduleId;
  mdMethodDef methodId;
  info::FunctionInfo info;
  configuration::StrictInterception interceptor;

  RejitInfo() {}
  RejitInfo(ModuleID m, mdMethodDef f, info::FunctionInfo i, configuration::StrictInterception interceptor) : moduleId(m), methodId(f), info(i), interceptor(interceptor) {}
};
