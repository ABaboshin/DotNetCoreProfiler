#pragma once

#include "MethodSignature.h"
#include "TypeInfo.h"
#include "info/GenericMethodSignature.h"

namespace info
{
    using namespace util;

    struct FunctionInfo {
        mdToken id;
        wstring name;
        TypeInfo type;
        MethodSignature signature{};
        GenericMethodSignature functionSpecSignature{};
        mdToken methodDefId;

        FunctionInfo()
            : id(0), name(""_W), type({}), methodDefId(0) {}

        FunctionInfo(mdToken id, wstring name, TypeInfo type,
            MethodSignature signature,
            GenericMethodSignature functionSpecSignature, mdToken methodDefId)
            : id(id),
            name(name),
            type(type),
            signature(signature),
            functionSpecSignature(functionSpecSignature),
            methodDefId(methodDefId) {}

        FunctionInfo(mdToken id, wstring name, TypeInfo type,
            MethodSignature signature)
            : id(id),
            name(name),
            type(type),
            signature(signature),
            methodDefId(0) {}

        static FunctionInfo GetFunctionInfo(const ComPtr<IMetaDataImport2>& metadataImport,
            const mdToken& token);

        TypeInfo ResolveParameterType(const TypeInfo& typeInfo);
    };
}