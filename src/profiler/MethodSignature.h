#pragma once

#include <iomanip>
#include "TypeInfo.h"
#include "util.h"

enum MethodArgumentTypeFlag
{
    TypeFlagByRef = 0x01,
    TypeFlagVoid = 0x02,
    TypeFlagBoxedType = 0x04
};

struct MethodArgument {
    ULONG offset;
    ULONG length;
    PCCOR_SIGNATURE pbBase;
    mdToken GetTypeTok(const ComPtr<IMetaDataEmit2> pEmit, mdAssemblyRef corLibRef) const;
    WSTRING GetTypeTokName(ComPtr<IMetaDataImport2>& pImport) const;
    int GetTypeFlags(unsigned& elementType) const;
};

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
    WSTRING str() const { return HexStr(pbBase, len); }
    MethodArgument GetRet() const { return  ret; }
    std::vector<MethodArgument> GetMethodArguments() const { return params; }
    HRESULT TryParse();
    bool operator ==(const MethodSignature& other) const {
        return memcmp(pbBase, other.pbBase, len);
    }
    CorCallingConvention CallingConvention() const {
        return CorCallingConvention(len == 0 ? 0 : pbBase[0]);
    }
    bool IsEmpty() const {
        return len == 0;
    }
};

bool ParseType(PCCOR_SIGNATURE& pbCur, PCCOR_SIGNATURE pbEnd);
bool ParseByte(PCCOR_SIGNATURE& pbCur, PCCOR_SIGNATURE pbEnd, unsigned char* pbOut);
bool ParseTypeDefOrRefEncoded(PCCOR_SIGNATURE& pbCur, PCCOR_SIGNATURE pbEnd,
    unsigned char* pIndexTypeOut, unsigned* pIndexOut);
bool ParseNumber(PCCOR_SIGNATURE& pbCur, PCCOR_SIGNATURE pbEnd, unsigned* pOut);

TypeInfo GetTypeInfo(const ComPtr<IMetaDataImport2>& metadata_import,
    const mdToken& token);

WSTRING GetSigTypeTokName(PCCOR_SIGNATURE& pbCur, const ComPtr<IMetaDataImport2>& pImport);