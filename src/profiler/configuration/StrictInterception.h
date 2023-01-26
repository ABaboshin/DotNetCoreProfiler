#pragma once

#include <unordered_set>
#include "TypeInfo.h"
#include "TargetMethod.h"
#include "util/util.h"

using namespace util;

namespace configuration
{
	struct StrictInterception {
		TargetMethod TargetMethod{};
		TypeInfo Interceptor{};
		int Priority = 0;

		StrictInterception(
			const configuration::TargetMethod& targetMethod, const ::configuration::TypeInfo& interceptor, int priority) :
			TargetMethod(targetMethod),
			Interceptor(interceptor),
			Priority(priority){}

		StrictInterception() {}
	};
}
