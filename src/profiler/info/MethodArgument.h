#pragma once
#include <vector>
#include "util/ComPtr.h"

namespace info
{
	struct MethodArgument
	{
	public:
		std::vector<BYTE> raw{};
		bool isRefType = false;
		BYTE typeDef = 0;

		MethodArgument(const std::vector<BYTE>& raw);
	};
}

