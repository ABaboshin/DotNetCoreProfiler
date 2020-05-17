#include "FunctionInfo.h"
#include "const/const.h"
#include <iostream>

namespace info
{
    std::vector<wstring> ExtractAttributes(const ComPtr<IMetaDataImport2>& metadataImport, mdToken token);

    FunctionInfo FunctionInfo::GetFunctionInfo(const ComPtr<IMetaDataImport2>& metadataImport, mdToken token) {

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
            functionName = util::ToRaw(genericInfo.Name);
            functionNameLength = functionName.size();
            methodSpecToken = token;
            methodDefToken = genericInfo.Id;
            finalSignature = genericInfo.Signature.raw;
            methodSpecSignature = util::ToRaw(rawSignature, rawSignatureLength);
        } break;
        default:
            break;
        }
        if (FAILED(hr) || functionNameLength == 0) {
            return {};
        }

        std::vector<wstring> attributes{};
        std::vector<wstring> parameterAttributes{};

        std::cout << "Method " << util::ToString(util::ToString(functionName, functionNameLength)) << std::endl;
        {
            auto res = ExtractAttributes(metadataImport, token);
            attributes.insert(attributes.end(), res.begin(), res.end());
        }

        {
            HCORENUM paramEnum = NULL;
            mdParamDef params[10];
            ULONG count, paramCount;
            bool first = true;
            HRESULT hr;

#define NumItems(s) (sizeof(s) / sizeof(s[0]))
            while (SUCCEEDED(hr = metadataImport->EnumParams(&paramEnum, token,
                params, NumItems(params), &count)) &&
                count > 0)
            {
                if (first)
                {
                    metadataImport->CountEnum(paramEnum, &paramCount);
                }
                for (ULONG i = 0; i < count; i++)
                {
                    mdMethodDef md;
                    ULONG num;
                    WCHAR paramName[_const::NameMaxSize];
                    ULONG nameLen;
                    DWORD flags;
                    VARIANT defValue;
                    DWORD dwCPlusFlags;
                    void const* pValue;
                    ULONG cbValue;

                    hr = metadataImport->GetParamProps(params[i], &md, &num, paramName, NumItems(paramName),
                        &nameLen, &flags, &dwCPlusFlags, &pValue, &cbValue);

                    auto res = ExtractAttributes(metadataImport, params[i]);
                    parameterAttributes.insert(parameterAttributes.end(), res.begin(), res.end());
                }
                first = false;
            }
            metadataImport->CloseEnum(paramEnum);
        }

        const auto typeInfo = TypeInfo::GetTypeInfo(metadataImport, parentToken);

        if (isGeneric)
        {
            return {methodSpecToken, util::ToString(functionName, functionNameLength), typeInfo, MethodSignature(finalSignature), GenericMethodSignature(methodSpecSignature), methodDefToken, attributes, parameterAttributes };
        }

        return { token, util::ToString(functionName, functionNameLength), typeInfo,
                MethodSignature(util::ToRaw(rawSignature,rawSignatureLength)), attributes, parameterAttributes };
    }

    std::vector<wstring> ExtractAttributes(const ComPtr<IMetaDataImport2>& metadataImport, mdToken token)
    {
        std::vector<wstring> attributes{};
        HRESULT hr;

#define NumItems(s) (sizeof(s) / sizeof(s[0]))

        HCORENUM customAttributeEnum = NULL;
        mdTypeRef customAttributes[10];
        ULONG count, totalCount = 1;
        while (SUCCEEDED(hr = metadataImport->EnumCustomAttributes(&customAttributeEnum, token, 0,
            customAttributes, NumItems(customAttributes), &count)) &&
            count > 0)
        {
            for (ULONG i = 0; i < count; i++, totalCount++)
            {
                mdToken     tkObj;                  // Attributed object.
                mdToken     tkType;                 // Type of the custom attribute.
                const BYTE* pValue;                // The custom value.
                ULONG       cbValue;                // Length of the custom value.
                MDUTF8CSTR     pMethName = 0;            // Name of custom attribute ctor, if any.
                PCCOR_SIGNATURE pSig = 0;             // Signature of ctor.
                ULONG       cbSig;                  // Size of the signature.

                std::vector<WCHAR> className(_const::NameMaxSize, (WCHAR)0);
                DWORD classNameLength = 0;

                hr = metadataImport->GetCustomAttributeProps( // S_OK or error.
                    customAttributes[i],                    // The attribute.
                    &tkObj,                     // The attributed object
                    &tkType,                    // The attributes type.
                    (const void**)&pValue,      // Put pointer to data here.
                    &cbValue);                  // Put size here.

                // Get the member name, and the parent token.
                switch (TypeFromToken(tkType))
                {
                case mdtMemberRef:
                    hr = metadataImport->GetNameFromToken(tkType, &pMethName);
                    hr = metadataImport->GetMemberRefProps(tkType, &tkType, 0, 0, 0, &pSig, &cbSig);
                    break;
                case mdtMethodDef:
                    hr = metadataImport->GetNameFromToken(tkType, &pMethName);
                    hr = metadataImport->GetMethodProps(tkType, &tkType, 0, 0, 0, 0, &pSig, &cbSig, 0, 0);
                    break;
                } // switch

                // Get the type name.
                switch (TypeFromToken(tkType))
                {
                case mdtTypeDef:
                    hr = metadataImport->GetTypeDefProps(tkType, &className[0], MAX_CLASS_NAME, &classNameLength, 0, 0);

                    attributes.push_back(util::ToString(className, classNameLength));
                    std::cout << "Attr mdtTypeDef " << util::ToString(util::ToString(className, classNameLength)) << std::endl;
                    break;
                case mdtTypeRef:
                    hr = metadataImport->GetTypeRefProps(tkType, 0, &className[0], MAX_CLASS_NAME, &classNameLength);
                    attributes.push_back(util::ToString(className, classNameLength));
                    std::cout << "Attr mdtTypeRef " << util::ToString(util::ToString(className, classNameLength)) << std::endl;
                    break;
                } // switch
            }
        }
        metadataImport->CloseEnum(customAttributeEnum);

        return attributes;
    }

    TypeInfo FunctionInfo::ResolveParameterType(const TypeInfo& typeInfo)
    {
        if (typeInfo.isGenericClassRef)
        {
            auto parameterType = Type.generics[typeInfo.genericRefNumber];
            parameterType.isRefType = typeInfo.isRefType;
            return parameterType;
        }

        if (typeInfo.isGenericMethodRef)
        {
            auto parameterType = FunctionSpecSignature.generics[typeInfo.genericRefNumber];
            parameterType.isRefType = typeInfo.isRefType;
            return parameterType;
        }

        return typeInfo;
    }
}