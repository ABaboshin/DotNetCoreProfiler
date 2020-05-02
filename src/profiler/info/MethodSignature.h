#pragma once

#include <iomanip>
#include "util/ComPtr.h"
#include "util/util.h"
#include "TypeInfo.h"

namespace info
{
    struct MethodSignature {
    public:
        std::vector<BYTE> raw{};

        MethodSignature() {}
        MethodSignature(std::vector<BYTE> raw) : raw(raw) {};

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

        std::vector<BYTE> GetRet();
    };
}