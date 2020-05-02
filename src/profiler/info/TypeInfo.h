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

        TypeInfo() : id(0), name(""_W) {}
        TypeInfo(mdToken id, wstring name, std::vector<BYTE> raw) : id(id), name(name), raw(raw) {}

        bool IsValid() const { return id != 0; }

        bool IsGeneric() const {
            return !raw.empty() && raw[0] == ELEMENT_TYPE_GENERICINST;
        }

    };

    TypeInfo GetTypeInfo(const ComPtr<IMetaDataImport2>& metadataImport,
        const mdToken& token);
}