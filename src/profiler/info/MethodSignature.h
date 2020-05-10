#pragma once

#include <iomanip>
#include "util/ComPtr.h"
#include "util/util.h"
#include "TypeInfo.h"
#include "info/MethodArgument.h"

namespace info
{
    struct MethodSignature {
    public:
        std::vector<BYTE> raw{};
        std::vector<BYTE> returnType{};
        ULONG numberOfArguments = 0;
        bool isGeneric = false;
        bool isInstanceMethod = false;
        std::vector<MethodArgument> arguments{};

        MethodSignature() {}
        MethodSignature(const std::vector<BYTE>& raw);
    };
}