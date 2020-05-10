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
        bool isBoxed = false;

        MethodArgument(const std::vector<BYTE>& raw);
    };
}