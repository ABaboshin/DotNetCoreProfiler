#pragma once

#include <unordered_set>
#include "TypeInfo.h"
#include "TargetMethod.h"
#include "util/util.h"

using namespace util;

namespace configuration
{
	struct StrictInterception {
		TargetMethod Target{};
		TypeInfo Interceptor{};

		StrictInterception(
			const TargetMethod& targetMethod, const ::configuration::TypeInfo& interceptor) :
			Target(targetMethod),
			Interceptor(interceptor) {}

		StrictInterception() {}
	};
}
