#include "FunctionInfo.h"
#include "const/const.h"

FunctionInfo GetFunctionInfo(const ComPtr<IMetaDataImport2>& metadata_import,
    const mdToken& token) {

    mdToken parent_token = mdTokenNil;
    WCHAR function_name[_const::NameMaxSize]{};
    DWORD function_name_len = 0;

    PCCOR_SIGNATURE raw_signature;
    ULONG raw_signature_len;

    HRESULT hr = E_FAIL;
    const auto token_type = TypeFromToken(token);
    switch (token_type) {
    case mdtMemberRef:
        hr = metadata_import->GetMemberRefProps(
            token, &parent_token, function_name, _const::NameMaxSize, &function_name_len,
            &raw_signature, &raw_signature_len);
        break;
    case mdtMethodDef:
        hr = metadata_import->GetMemberProps(
            token, &parent_token, function_name, _const::NameMaxSize, &function_name_len,
            nullptr, &raw_signature, &raw_signature_len, nullptr, nullptr,
            nullptr, nullptr, nullptr);
        break;
    case mdtMethodSpec: {
        hr = metadata_import->GetMethodSpecProps(
            token, &parent_token, &raw_signature, &raw_signature_len);
        if (FAILED(hr)) {
            return {};
        }
        auto generic_info = GetFunctionInfo(metadata_import, parent_token);
        memcpy(function_name, generic_info.name.c_str(),
            sizeof(WCHAR) * (generic_info.name.length() + 1));
        function_name_len = (DWORD)(generic_info.name.length() + 1);
    } break;
    default:
        break;
    }
    if (FAILED(hr) || function_name_len == 0) {
        return {};
    }

    // parent_token could be: TypeDef, TypeRef, TypeSpec, ModuleRef, MethodDef
    const auto type_info = GetTypeInfo(metadata_import, parent_token);

    return { token, function_name, type_info,
            MethodSignature(raw_signature,raw_signature_len) };
}