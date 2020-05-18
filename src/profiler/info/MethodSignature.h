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
        std::vector<BYTE> Raw{};
        TypeInfo ReturnType{};
        std::vector<TypeInfo> Arguments{};

        MethodSignature() {}
        MethodSignature(std::vector<BYTE> raw);

        void ParseArguments();

        COR_SIGNATURE CallingConvention() const { return Raw.empty() ? 0 : Raw[0]; }

        bool IsInstanceMethod() const {
            return (CallingConvention() & IMAGE_CEE_CS_CALLCONV_HASTHIS) != 0;
        }

        bool IsGeneric() const {
            return Raw.size() > 2 && (CallingConvention() & IMAGE_CEE_CS_CALLCONV_GENERIC) != 0;
        }

        ULONG NumberOfArguments() const {
            if (IsGeneric())
            {
                return Raw[2];
            }

            if (Raw.size() > 1)
            {
                return Raw[1];
            }

            return 0;
        }
    };
}