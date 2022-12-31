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
		int Priority = 0;

		StrictInterception(
			const TargetMethod& targetMethod, const ::configuration::TypeInfo& interceptor, int priority) :
			Target(targetMethod),
			Interceptor(interceptor),
			Priority(priority){}

		StrictInterception() {}
	};
}
