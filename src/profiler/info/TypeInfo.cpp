#include "TypeInfo.h"
#include "const/const.h"
#include "util/helpers.h"
#include <iostream>

namespace info
{
    TypeInfo GetTypeInfo(const ComPtr<IMetaDataImport2>& metadataImport,
        const mdToken& token) {
        mdToken parent_token = mdTokenNil;
        WCHAR type_name[_const::NameMaxSize]{};
        DWORD type_name_len = 0;

        HRESULT hr = E_FAIL;
        const auto token_type = TypeFromToken(token);
        switch (token_type) {
        case mdtTypeDef:
            hr = metadataImport->GetTypeDefProps(token, type_name, _const::NameMaxSize,
                &type_name_len, nullptr, nullptr);
            break;
        case mdtTypeRef:
            hr = metadataImport->GetTypeRefProps(token, &parent_token, type_name,
                _const::NameMaxSize, &type_name_len);
            break;
        case mdtTypeSpec: {
            PCCOR_SIGNATURE signature{};
            ULONG signature_length{};

            hr = metadataImport->GetTypeSpecFromToken(token, &signature,
                &signature_length);

            if (FAILED(hr) || signature_length < 3) {
                return {};
            }

            auto raw = std::vector<BYTE>(&signature[0], &signature[signature_length]);

            if (signature[0] & ELEMENT_TYPE_GENERICINST) {
                mdToken type_token;
                CorSigUncompressToken(&signature[2], &type_token);
                auto xxx = GetTypeInfo(metadataImport, type_token);
                std::cout << "Generic " << util::ToString(xxx.name) << std::endl;
            }

            return { token, GetSigTypeTokName(signature, metadataImport), raw };
        } break;
        case mdtModuleRef:
            metadataImport->GetModuleRefProps(token, type_name, _const::NameMaxSize,
                &type_name_len);
            break;
        case mdtMemberRef:
            return GetFunctionInfo(metadataImport, token).type;
            break;
        case mdtMethodDef:
            return GetFunctionInfo(metadataImport, token).type;
            break;
        }
        if (FAILED(hr) || type_name_len == 0) {
            return {};
        }

        return { token, type_name, {} };
    }
}