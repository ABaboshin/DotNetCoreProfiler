#pragma once

#include "util/ComPtr.h"
#include "util/util.h"
#include "info/TypeInfo.h"

namespace info {
	struct GenericMethodSignature
	{
		std::vector<BYTE> raw{};
		std::vector<TypeInfo> generics{};
		GenericMethodSignature(std::vector<BYTE> raw);
		GenericMethodSignature() {}
	};
}

