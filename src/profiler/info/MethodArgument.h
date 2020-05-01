#pragma once

#include "util/ComPtr.h"
#include "util/util.h"

using namespace util;

struct MethodArgument {
    ULONG offset;
    ULONG length;
    PCCOR_SIGNATURE pbBase;
    mdToken GetTypeTok(const ComPtr<IMetaDataEmit2> pEmit, mdAssemblyRef corLibRef) const;
    wstring GetTypeTokName(ComPtr<IMetaDataImport2>& pImport) const;
    int GetTypeFlags(unsigned& elementType) const;
    std::vector<BYTE> GetRaw() const {
        return std::vector<BYTE>(&pbBase[offset], &pbBase[offset + length]);
    }
    bool IsGeneric() const {
        return pbBase[offset] == ELEMENT_TYPE_GENERICINST;
    }
};