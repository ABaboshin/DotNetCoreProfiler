#pragma once

#include <vector>
#include "util/util.h"
#include "StrictInterception.h"
#include "TargetMethod.h"

namespace configuration
{
	using namespace util;

  struct InstrumentationConfiguration
  {
    bool Trace;
    std::vector<wstring> Parameters{};
    std::vector<wstring> Variables{};
    std::vector<int> Offsets{};
    wstring TraceName;
    std::vector<StrictInterception> Interceptors;

    InstrumentationConfiguration(bool trace, const std::vector<wstring> &parameters, const std::vector<wstring> &variables, const wstring &name, const std::vector<StrictInterception> &interceptors) : Trace(trace),
    Variables(variables), Parameters(parameters), TraceName(name), Interceptors(interceptors)
    {
    }

    InstrumentationConfiguration() {}
  };
}
