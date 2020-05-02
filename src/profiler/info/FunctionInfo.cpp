#include "FunctionInfo.h"
#include "const/const.h"

namespace info
{
    FunctionInfo FunctionInfo::GetFunctionInfo(const ComPtr<IMetaDataImport2>& metadataImport,
        const mdToken& token) {

        mdToken parentToken = mdTokenNil;
        WCHAR functionName[_const::NameMaxSize]{};
        DWORD functionNameLength = 0;

        PCCOR_SIGNATURE rawSignature;
        ULONG rawSignatureLength;

        bool isGeneric = false;

        HRESULT hr = E_FAIL;
        const auto token_type = TypeFromToken(token);
        switch (token_type) {
        case mdtMemberRef:
            hr = metadataImport->GetMemberRefProps(
                token, &parentToken, functionName, _const::NameMaxSize, &functionNameLength,
                &rawSignature, &rawSignatureLength);
            break;
        case mdtMethodDef:
            hr = metadataImport->GetMemberProps(
                token, &parentToken, functionName, _const::NameMaxSize, &functionNameLength,
                nullptr, &rawSignature, &rawSignatureLength, nullptr, nullptr,
                nullptr, nullptr, nullptr);
            break;
        case mdtMethodSpec: {
            hr = metadataImport->GetMethodSpecProps(
                token, &parentToken, &rawSignature, &rawSignatureLength);
            if (FAILED(hr)) {
                return {};
            }
            auto genericInfo = GetFunctionInfo(metadataImport, parentToken);
            memcpy(functionName, genericInfo.name.c_str(),
                sizeof(WCHAR) * (genericInfo.name.length() + 1));
            functionNameLength = (DWORD)(genericInfo.name.length() + 1);
        } break;
        default:
            break;
        }
        if (FAILED(hr) || functionNameLength == 0) {
            return {};
        }

        // parent_token could be: TypeDef, TypeRef, TypeSpec, ModuleRef, MethodDef
        const auto typeInfo = GetTypeInfo(metadataImport, parentToken);

        return { token, functionName, typeInfo,
                MethodSignature(rawSignature,rawSignatureLength) };
    }
}