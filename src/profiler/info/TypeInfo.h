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

        TypeInfo() : id(0), name(""_W) {}
        TypeInfo(mdToken id, wstring name) : id(id), name(name) {}

        static TypeInfo GetTypeInfo(const ComPtr<IMetaDataImport2>& metadataImport,
            const mdToken& token);
    };
}