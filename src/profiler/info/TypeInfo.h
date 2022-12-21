#pragma once

#include "cor.h"
#include "util/util.h"
#include "util/ComPtr.h"

namespace info
{
    using namespace util;

    struct TypeInfo {
        mdToken Id;
        wstring Name;
        std::vector<BYTE> Raw{};

        std::vector<TypeInfo> Generics{};

        bool IsRefType = false;
        BYTE TypeDef = 0;
        bool IsBoxed = false;
        bool IsVoid = false;

        bool IsValueType = false;
        bool IsAbstract = false;
        bool IsSealed = false;

        bool IsGenericClassRef = false;
        bool IsGenericMethodRef = false;
        ULONG GenericRefNumber = 0;
        mdTypeSpec TypeSpec;
        ULONG32 TokenType;

        TypeInfo() : Id(0), Name(""_W) {}
        TypeInfo(mdToken id, wstring name, const std::vector<BYTE>& raw, bool isValueType, bool isAbstract, bool isSealed, mdTypeSpec typeSpec, ULONG32 tokenType, bool isGeneric) :
            Id(id), Name(name), Raw(raw), IsValueType(isValueType), IsAbstract(isAbstract), IsSealed(isSealed), TypeSpec(typeSpec), TokenType(tokenType), IsGenericClassRef(isGeneric) {}
        TypeInfo(const std::vector<BYTE>& raw);

        void TryParseGeneric();

        static TypeInfo GetTypeInfo(const ComPtr<IMetaDataImport2>& metadataImport, mdToken token);
    };
}