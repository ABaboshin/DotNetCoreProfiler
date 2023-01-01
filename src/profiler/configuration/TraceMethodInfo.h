#pragma once

#include <vector>
#include "util/util.h"
#include "TargetMethod.h"

namespace configuration
{
	using namespace util;

  struct TraceMethodInfo
  {
    TargetMethod TargetMethod{};
    std::vector<wstring> Parameters{};
    wstring Name;

    TraceMethodInfo(const configuration::TargetMethod& targetMethod, const std::vector<wstring> &parameters, const wstring& name) : TargetMethod(targetMethod),
                                                                                                Parameters(parameters), Name(name) {}

    TraceMethodInfo() {}
  };
}
