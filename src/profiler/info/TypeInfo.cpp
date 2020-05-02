#include "TypeInfo.h"
#include "const/const.h"
#include "util/helpers.h"
#include <iostream>

namespace info
{
    TypeInfo TypeInfo::GetTypeInfo(const ComPtr<IMetaDataImport2>& metadataImport,
        const mdToken& token) {
        mdToken parent_token = mdTokenNil;

        std::vector<WCHAR> typeName(_const::NameMaxSize, (WCHAR)0);
        DWORD length = 0;

        HRESULT hr = E_FAIL;
        const auto token_type = TypeFromToken(token);
        switch (token_type) {
        case mdtTypeDef:
            hr = metadataImport->GetTypeDefProps(token, &typeName[0], _const::NameMaxSize,
                &length, nullptr, nullptr);
            break;
        case mdtTypeRef:
            hr = metadataImport->GetTypeRefProps(token, &parent_token, &typeName[0],
                _const::NameMaxSize, &length);
            break;
        case mdtTypeSpec: {
            PCCOR_SIGNATURE signature{};
            ULONG signature_length{};

            hr = metadataImport->GetTypeSpecFromToken(token, &signature,
                &signature_length);

            if (FAILED(hr) || signature_length < 3) {
                return {};
            }

            if (signature[0] & ELEMENT_TYPE_GENERICINST) {
                mdToken typeToken;
                CorSigUncompressToken(&signature[2], &typeToken);
                return GetTypeInfo(metadataImport, typeToken);
            }

        } break;
        case mdtModuleRef:
            metadataImport->GetModuleRefProps(token, &typeName[0], _const::NameMaxSize,
                &length);
            break;
        case mdtMemberRef:
            return FunctionInfo::GetFunctionInfo(metadataImport, token).type;
            break;
        case mdtMethodDef:
            return FunctionInfo::GetFunctionInfo(metadataImport, token).type;
            break;
        }
        if (FAILED(hr) || length == 0) {
            return {};
        }

        return { token, util::ToString(typeName) };
    }
}