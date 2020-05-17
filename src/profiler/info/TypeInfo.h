#pragma once

#include "cor.h"
#include "util/util.h"
#include "util/ComPtr.h"

namespace info
{
    using namespace util;

    struct TypeInfo {
        mdToken id;
        wstring name;
        std::vector<BYTE> raw{};

        std::vector<TypeInfo> generics{};

        bool isRefType = false;
        BYTE typeDef = 0;
        bool isBoxed = false;
        bool isVoid = false;

        bool isGenericClassRef = false;
        bool isGenericMethodRef = false;
        ULONG genericRefNumber = 0;

        TypeInfo() : id(0), name(""_W) {}
        TypeInfo(mdToken id, wstring name, const std::vector<BYTE>& raw) : id(id), name(name), raw(raw) {}
        TypeInfo(const std::vector<BYTE>& raw);

        void TryParseGeneric();

        static TypeInfo GetTypeInfo(const ComPtr<IMetaDataImport2>& metadataImport, mdToken token);
    };
}