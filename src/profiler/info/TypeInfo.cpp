#include <iostream>
#include <cor.h>
#include "parser.h"
#include "TypeInfo.h"
#include "const/const.h"
#include "util/helpers.h"


namespace info
{
    TypeInfo::TypeInfo(const std::vector<BYTE>& raw) : Raw(raw)
    {
        IsRefType = raw.size() > 0 && raw[0] == ELEMENT_TYPE_BYREF;

        IsVoid = raw.size() > 0 && raw[0] == ELEMENT_TYPE_VOID;

        auto shift = IsRefType ? 1 : 0;

        switch (raw[IsRefType]) {
        case ELEMENT_TYPE_VOID:
        case ELEMENT_TYPE_BOOLEAN:
        case ELEMENT_TYPE_CHAR:
        case ELEMENT_TYPE_I1:
        case ELEMENT_TYPE_U1:
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
        case ELEMENT_TYPE_R4:
        case ELEMENT_TYPE_R8:
        case ELEMENT_TYPE_I:
        case ELEMENT_TYPE_U:
        case ELEMENT_TYPE_STRING:
            //case ELEMENT_TYPE_VALUETYPE:
            TypeDef = raw[IsRefType];
            IsBoxed = true;
            break;
        case ELEMENT_TYPE_MVAR:
        {
            IsGenericMethodRef = true;
            auto iter = this->Raw.begin();
            std::advance(iter, 1 + shift);
            ParseNumber(iter, GenericRefNumber);
        }
        break;
        case ELEMENT_TYPE_VAR:
        {
            IsGenericClassRef = true;
            auto iter = this->Raw.begin();
            std::advance(iter, 1 + shift);
            ParseNumber(iter, GenericRefNumber);
        }
        break;
        default:
            TypeDef = ELEMENT_TYPE_OBJECT;
            IsBoxed = false;
            break;
        }
    }

    void TypeInfo::TryParseGeneric()
    {
        auto iter = Raw.begin();
        ULONG elemenType = 0;
        ParseNumber(iter, elemenType); // => ELEMENT_TYPE_GENERICINST
        ParseNumber(iter, elemenType); // => ELEMENT_TYPE_CLASS, ELEMENT_TYPE_VALUETYPE

        if (elemenType != ELEMENT_TYPE_CLASS && elemenType != ELEMENT_TYPE_VALUETYPE)
        {
            return;
        }

        ParseNumber(iter, elemenType); // => token

        ULONG number = 0;

        ParseNumber(iter, number); // => number of generic arguments

        for (size_t i = 0; i < number; i++)
        {
            auto begin = iter;
            if (!ParseType(iter))
            {
                break;
            }

            Generics.push_back(TypeInfo(std::vector<BYTE>(begin, iter)));
        }
    }

    TypeInfo TypeInfo::GetTypeInfo(const ComPtr<IMetaDataImport2>& metadataImport, mdToken token) {
        std::vector<WCHAR> typeName(MAX_CLASS_NAME, (WCHAR)0);
        DWORD typeNameLength = 0;
        DWORD typeFlags;
        mdToken typeExtends = mdTokenNil;
        mdToken parentTypeToken = mdTokenNil;
        TypeInfo parentTypeInfo;
        TypeInfo extendsInfo;

        bool isValueType = false;
        bool isAbstract = false;
        bool isSealed = false;
        bool isGeneric = false;

        HRESULT hr = E_FAIL;
        const auto tokenType = TypeFromToken(token);
        switch (tokenType) {
        case mdtTypeDef:
            hr = metadataImport->GetTypeDefProps(token, &typeName[0], MAX_CLASS_NAME, &typeNameLength, &typeFlags, &typeExtends);
            metadataImport->GetNestedClassProps(token, &parentTypeToken);

            if (parentTypeToken != mdTokenNil)
            {
                parentTypeInfo = GetTypeInfo(metadataImport, parentTypeToken);
            }

            if (typeExtends != mdTokenNil)
            {
                extendsInfo = GetTypeInfo(metadataImport, typeExtends);
                isValueType =
                    extendsInfo.Name == "System.ValueType"_W || extendsInfo.Name == "System.Enum"_W;
            }

            isAbstract = IsTdAbstract(typeFlags);
            isSealed = IsTdSealed(typeFlags);

            break;
        case mdtTypeRef:
            hr = metadataImport->GetTypeRefProps(token, nullptr, &typeName[0],
                MAX_CLASS_NAME, &typeNameLength);
            break;
        case mdtTypeSpec:
        {
            PCCOR_SIGNATURE signature{};
            ULONG signatureLength{};

            hr = metadataImport->GetTypeSpecFromToken(token, &signature, &signatureLength);

            if (FAILED(hr) || signatureLength < 3) {
                return {};
            }

            if (signature[0] & ELEMENT_TYPE_GENERICINST) {
                mdToken typeToken;
                auto length = CorSigUncompressToken(&signature[2], &typeToken);
                auto ti = GetTypeInfo(metadataImport, typeToken);
                ti.Raw = util::ToRaw(signature, signatureLength);
                ti.TryParseGeneric();

                return { ti.Id, ti.Name, {}, ti.IsValueType, ti.IsAbstract, ti.IsSealed, token, tokenType, ti.IsGenericClassRef, ti.ParentTypeInfo };

                return ti;
            }
        }
        break;
        case mdtModuleRef:
            metadataImport->GetModuleRefProps(token, &typeName[0], MAX_CLASS_NAME, &typeNameLength);
            break;
        case mdtMemberRef:
            return FunctionInfo::GetFunctionInfo(metadataImport, token).Type;
            break;
        case mdtMethodDef:
            return FunctionInfo::GetFunctionInfo(metadataImport, token).Type;
            break;
        }

        if (FAILED(hr) || typeNameLength == 0) {
            return {};
        }

        const auto name = util::ToString(typeName, typeNameLength);
        const auto genericTokenIndex = name.rfind("`"_W);
        if (genericTokenIndex != std::string::npos)
        {
            const auto idxFromRight = name.length() - genericTokenIndex - 1;
            isGeneric = idxFromRight == 1 || idxFromRight == 2;
        }

        return { token, util::ToString(typeName, typeNameLength), {}, isValueType, isAbstract, isSealed, mdTypeSpecNil, tokenType, isGeneric, std::make_shared<TypeInfo>(parentTypeInfo)};
    }
}