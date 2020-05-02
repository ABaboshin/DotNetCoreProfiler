#pragma once

#include "MethodSignature.h"
#include "TypeInfo.h"

namespace info
{
    using namespace util;

    struct FunctionInfo {
        mdToken id;
        wstring name;
        TypeInfo type;
        BOOL isGeneric;
        MethodSignature signature;
        MethodSignature functionSpecSignature;
        mdToken methodDefId;

        FunctionInfo()
            : id(0), name(""_W), type({}), isGeneric(false), methodDefId(0) {}

        FunctionInfo(mdToken id, wstring name, TypeInfo type,
            MethodSignature signature,
            MethodSignature functionSpecSignature, mdToken methodDefId)
            : id(id),
            name(name),
            type(type),
            isGeneric(true),
            signature(signature),
            functionSpecSignature(functionSpecSignature),
            methodDefId(methodDefId) {}

        FunctionInfo(mdToken id, wstring name, TypeInfo type,
            MethodSignature signature)
            : id(id),
            name(name),
            type(type),
            isGeneric(false),
            signature(signature),
            methodDefId(0) {}

        static FunctionInfo GetFunctionInfo(const ComPtr<IMetaDataImport2>& metadataImport,
            const mdToken& token);
    };
}