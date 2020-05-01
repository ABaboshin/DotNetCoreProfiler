#pragma once

#include <iomanip>
#include "util/ComPtr.h"
#include "TypeInfo.h"
#include "util/util.h"
#include "MethodArgument.h"

struct MethodSignature {
private:
    PCCOR_SIGNATURE pbBase;
    unsigned len;
    ULONG numberOfTypeArguments = 0;
    ULONG numberOfArguments = 0;
    MethodArgument ret{};
    std::vector<MethodArgument> params;
public:
    MethodSignature() : pbBase(nullptr), len(0) {}
    MethodSignature(PCCOR_SIGNATURE pb, unsigned cbBuffer) {
        pbBase = pb;
        len = cbBuffer;
    };
    ULONG NumberOfTypeArguments() const { return numberOfTypeArguments; }
    ULONG NumberOfArguments() const { return numberOfArguments; }
    MethodArgument GetRet() const { return  ret; }
    std::vector<MethodArgument> GetMethodArguments() const { return params; }
    HRESULT TryParse();
    bool operator ==(const MethodSignature& other) const {
        return memcmp(pbBase, other.pbBase, len);
    }
    COR_SIGNATURE CallingConvention() const {
        return len == 0 ? 0 : pbBase[0];
    }
    bool IsEmpty() const {
        return len == 0;
    }

    bool IsInstanceMethod() const {
        return (CallingConvention() & IMAGE_CEE_CS_CALLCONV_HASTHIS) != 0;
    }

    std::vector<BYTE> GetRaw() const {
        return std::vector<BYTE>(&pbBase[0], &pbBase[len]);
    }
};
