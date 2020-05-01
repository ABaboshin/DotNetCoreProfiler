#include "TypeInfo.h"
#include "const/const.h"
#include "util/helpers.h"

namespace info
{
    TypeInfo GetTypeInfo(const ComPtr<IMetaDataImport2>& metadata_import,
        const mdToken& token) {
        mdToken parent_token = mdTokenNil;
        WCHAR type_name[_const::NameMaxSize]{};
        DWORD type_name_len = 0;

        HRESULT hr = E_FAIL;
        const auto token_type = TypeFromToken(token);
        switch (token_type) {
        case mdtTypeDef:
            //std::cout << "mdtTypeDef" << std::endl;
            hr = metadata_import->GetTypeDefProps(token, type_name, _const::NameMaxSize,
                &type_name_len, nullptr, nullptr);
            break;
        case mdtTypeRef:
            //std::cout << "mdtTypeRef" << std::endl;
            hr = metadata_import->GetTypeRefProps(token, &parent_token, type_name,
                _const::NameMaxSize, &type_name_len);
            break;
        case mdtTypeSpec: {
            //std::cout << "mdtTypeSpec" << std::endl;
            PCCOR_SIGNATURE signature{};
            ULONG signature_length{};

            hr = metadata_import->GetTypeSpecFromToken(token, &signature,
                &signature_length);

            if (FAILED(hr) || signature_length < 3) {
                return {};
            }

            auto raw = std::vector<BYTE>(&signature[0], &signature[signature_length]);

            return { token, GetSigTypeTokName(signature, metadata_import), raw };
        } break;
        case mdtModuleRef:
            //std::cout << "mdtModuleRef" << std::endl;
            metadata_import->GetModuleRefProps(token, type_name, _const::NameMaxSize,
                &type_name_len);
            break;
        case mdtMemberRef:
            return GetFunctionInfo(metadata_import, token).type;
            break;
        case mdtMethodDef:
            return GetFunctionInfo(metadata_import, token).type;
            break;
        }
        if (FAILED(hr) || type_name_len == 0) {
            return {};
        }

        return { token, type_name, {} };
    }
}