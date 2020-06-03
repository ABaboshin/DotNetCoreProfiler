#pragma once

#include <unordered_set>
#include "MethodSignature.h"
#include "TypeInfo.h"
#include "info/GenericMethodSignature.h"

namespace info
{
    using namespace util;

    struct FunctionInfo {
        mdToken Id;
        wstring Name;
        TypeInfo Type{};
        MethodSignature Signature{};
        GenericMethodSignature FunctionSpecSignature{};
        mdToken MethodDefId;
        std::unordered_set<wstring> Attributes{};

        FunctionInfo()
            : Id(0), Name(""_W), MethodDefId(0) {}

        FunctionInfo(mdToken id, wstring name, TypeInfo type,
            MethodSignature signature,
            GenericMethodSignature functionSpecSignature, mdToken methodDefId,
            const std::unordered_set<wstring>& attributes)
            : Id(id),
            Name(name),
            Type(type),
            Signature(signature),
            FunctionSpecSignature(functionSpecSignature),
            MethodDefId(methodDefId),
            Attributes(attributes) {}

        FunctionInfo(mdToken id, wstring name, TypeInfo type,
            MethodSignature signature,
            const std::unordered_set<wstring>& attributes)
            : Id(id),
            Name(name),
            Type(type),
            Signature(signature),
            MethodDefId(0),
            Attributes(attributes) {}

        static FunctionInfo GetFunctionInfo(const ComPtr<IMetaDataImport2>& metadataImport, mdToken token);

        TypeInfo ResolveParameterType(const TypeInfo& typeInfo) const;
    };

    std::unordered_set<wstring> ExtractAttributes(const ComPtr<IMetaDataImport2>& metadataImport, mdToken token);
}