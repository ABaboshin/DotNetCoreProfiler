#pragma once

#include <iomanip>
#include "util/ComPtr.h"
#include "util/util.h"
#include "info/TypeInfo.h"

namespace info
{
    struct MethodSignature {
        size_t argumentsOffset = 0;
    public:
        std::vector<BYTE> raw{};
        TypeInfo ret{};
        std::vector<TypeInfo> arguments{};

        MethodSignature() {}
        MethodSignature(std::vector<BYTE> raw);

        void ParseArguments();

        COR_SIGNATURE CallingConvention() const { return raw.empty() ? 0 : raw[0]; }

        bool IsInstanceMethod() const {
            return (CallingConvention() & IMAGE_CEE_CS_CALLCONV_HASTHIS) != 0;
        }

        bool IsGeneric() const {
            return raw.size() > 2 && (CallingConvention() & IMAGE_CEE_CS_CALLCONV_GENERIC) != 0;
        }

        ULONG NumberOfArguments() const {
            if (IsGeneric())
            {
                return raw[2];
            }

            if (raw.size() > 1)
            {
                return raw[1];
            }

            return 0;
        }
    };
}