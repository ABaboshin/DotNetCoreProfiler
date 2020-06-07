#pragma once

#include "TypeInfo.h"
#include "TargetMethod.h"
#include "util/util.h"

using namespace util;

namespace configuration
{
	struct MethodFinder {
		TargetMethod Target{};
		TypeInfo Finder{};

		MethodFinder(const TargetMethod& targetMethod, const ::configuration::TypeInfo& finder) :
			Target(targetMethod),
			Finder(finder) {}

		MethodFinder() {}
	};
}