#pragma once

#include "util/ComPtr.h"
#include "util/util.h"
#include "info/TypeInfo.h"

namespace info {
	struct GenericMethodSignature
	{
		std::vector<BYTE> Raw{};
		std::vector<TypeInfo> Generics{};
		GenericMethodSignature(std::vector<BYTE> raw);
		GenericMethodSignature() {}
	};
}

