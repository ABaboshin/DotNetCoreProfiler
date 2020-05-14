#include "FunctionInfo.h"
#include "const/const.h"

namespace info
{
    FunctionInfo FunctionInfo::GetFunctionInfo(const ComPtr<IMetaDataImport2>& metadataImport,
        const mdToken& token) {

        mdToken parentToken = mdTokenNil;
        mdToken methodSpecToken = mdTokenNil;
        mdToken methodDefToken = mdTokenNil;

        std::vector<WCHAR> functionName(_const::NameMaxSize, (WCHAR)0);
        DWORD functionNameLength = 0;

        PCCOR_SIGNATURE rawSignature;
        ULONG rawSignatureLength;

        std::vector<BYTE> finalSignature{};
        std::vector<BYTE> methodSpecSignature{};

        bool isGeneric = false;

        HRESULT hr = E_FAIL;
        const auto token_type = TypeFromToken(token);
        switch (token_type) {
        case mdtMemberRef:
            hr = metadataImport->GetMemberRefProps(
                token, &parentToken, &functionName[0], _const::NameMaxSize, &functionNameLength,
                &rawSignature, &rawSignatureLength);
            break;
        case mdtMethodDef:
            hr = metadataImport->GetMemberProps(
                token, &parentToken, &functionName[0], _const::NameMaxSize, &functionNameLength,
                nullptr, &rawSignature, &rawSignatureLength, nullptr, nullptr,
                nullptr, nullptr, nullptr);
            break;
        case mdtMethodSpec: {
            isGeneric = true;
            hr = metadataImport->GetMethodSpecProps(
                token, &parentToken, &rawSignature, &rawSignatureLength);
            if (FAILED(hr)) {
                return {};
            }
            auto genericInfo = GetFunctionInfo(metadataImport, parentToken);
            functionName = util::ToRaw(genericInfo.name);
            functionNameLength = functionName.size();
            methodSpecToken = token;
            methodDefToken = genericInfo.id;
            finalSignature = genericInfo.signature.raw;
            methodSpecSignature = util::ToRaw(rawSignature, rawSignatureLength);
        } break;
        default:
            break;
        }
        if (FAILED(hr) || functionNameLength == 0) {
            return {};
        }

        const auto typeInfo = TypeInfo::GetTypeInfo(metadataImport, parentToken);

        if (isGeneric)
        {
            return {methodSpecToken, util::ToString(functionName, functionNameLength), typeInfo, MethodSignature(finalSignature), MethodSignature(methodSpecSignature), methodDefToken};
        }

        return { token, util::ToString(functionName, functionNameLength), typeInfo,
                MethodSignature(util::ToRaw(rawSignature,rawSignatureLength)) };
    }
}