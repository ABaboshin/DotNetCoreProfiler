#include <cstring>
#include <iostream>
#include <vector>

#include "info/AssemblyInfo.h"
#include "info/ModuleInfo.h"
#include "const/const.h"
#include "helpers.h"
#include "util.h"

namespace util
{
    wstring GetSigTypeTokName(PCCOR_SIGNATURE& pbCur, const ComPtr<IMetaDataImport2>& metadaImport)
    {
        wstring tokenName = ""_W;
        bool ref_flag = false;
        if (*pbCur == ELEMENT_TYPE_BYREF)
        {
            pbCur++;
            ref_flag = true;
        }

        switch (*pbCur) {
        case  ELEMENT_TYPE_BOOLEAN:
            tokenName = _const::SystemBoolean;
            pbCur++;
            break;
        case  ELEMENT_TYPE_CHAR:
            tokenName = _const::SystemChar;
            pbCur++;
            break;
        case  ELEMENT_TYPE_I1:
            tokenName = _const::SystemByte;
            pbCur++;
            break;
        case  ELEMENT_TYPE_U1:
            tokenName = _const::SystemSByte;
            pbCur++;
            break;
        case  ELEMENT_TYPE_U2:
            tokenName = _const::SystemUInt16;
            pbCur++;
            break;
        case  ELEMENT_TYPE_I2:
            tokenName = _const::SystemInt16;
            pbCur++;
            break;
        case  ELEMENT_TYPE_I4:
            tokenName = _const::SystemInt32;
            pbCur++;
            break;
        case  ELEMENT_TYPE_U4:
            tokenName = _const::SystemUInt32;
            pbCur++;
            break;
        case  ELEMENT_TYPE_I8:
            tokenName = _const::SystemInt64;
            pbCur++;
            break;
        case  ELEMENT_TYPE_U8:
            tokenName = _const::SystemUInt64;
            pbCur++;
            break;
        case  ELEMENT_TYPE_R4:
            tokenName = _const::SystemSingle;
            pbCur++;
            break;
        case  ELEMENT_TYPE_R8:
            tokenName = _const::SystemDouble;
            pbCur++;
            break;
        case  ELEMENT_TYPE_I:
            tokenName = _const::SystemIntPtr;
            pbCur++;
            break;
        case  ELEMENT_TYPE_U:
            tokenName = _const::SystemUIntPtr;
            pbCur++;
            break;
        case  ELEMENT_TYPE_STRING:
            tokenName = _const::SystemString;
            pbCur++;
            break;
        case  ELEMENT_TYPE_OBJECT:
            tokenName = _const::SystemObject;
            pbCur++;
            break;
        case  ELEMENT_TYPE_CLASS:
        case  ELEMENT_TYPE_VALUETYPE:
        {
            pbCur++;
            mdToken token;
            pbCur += CorSigUncompressToken(pbCur, &token);
            tokenName = GetTypeInfo(metadaImport, token).name;
            break;
        }
        case  ELEMENT_TYPE_SZARRAY:
        {
            pbCur++;
            tokenName = GetSigTypeTokName(pbCur, metadaImport) + "[]"_W;
            break;
        }
        case  ELEMENT_TYPE_GENERICINST:
        {
            pbCur++;
            tokenName = GetSigTypeTokName(pbCur, metadaImport);
            tokenName += "["_W;
            ULONG num = 0;
            pbCur += CorSigUncompressData(pbCur, &num);
            for (ULONG i = 0; i < num; i++) {
                tokenName += GetSigTypeTokName(pbCur, metadaImport);
                if (i != num - 1) {
                    tokenName += ","_W;
                }
            }
            tokenName += "]"_W;
            break;
        }
        case  ELEMENT_TYPE_MVAR:
        {
            pbCur++;
            ULONG num = 0;
            pbCur += CorSigUncompressData(pbCur, &num);
            tokenName = "!!"_W + ToWSTRING(std::to_string(num));
            break;
        }
        case  ELEMENT_TYPE_VAR:
        {
            pbCur++;
            ULONG num = 0;
            pbCur += CorSigUncompressData(pbCur, &num);
            tokenName = "!"_W + ToWSTRING(std::to_string(num));
            break;
        }
        default:
            break;
        }

        if (ref_flag) {
            tokenName += "&"_W;
        }

        return tokenName;
    }

    HRESULT CreateAssemblyRef(const ComPtr<IMetaDataAssemblyEmit> metadataAssemblyEmit, mdAssemblyRef* libRef, const std::vector<BYTE>& public_key, ASSEMBLYMETADATA metadata, const wstring& assemblyName) {
        HRESULT hr = metadataAssemblyEmit->DefineAssemblyRef(
            (void*)public_key.data(),
            (ULONG)public_key.size(),
            assemblyName.c_str(), &metadata, NULL, 0, 0,
            libRef);

        return hr;
    }

    void GetMsCorLibRef(HRESULT& hr, const ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdModuleRef& libRef)
    {
        ASSEMBLYMETADATA metadata{};
        metadata.usMajorVersion = 4;
        metadata.usMinorVersion = 0;
        metadata.usBuildNumber = 0;
        metadata.usRevisionNumber = 0;

        hr = CreateAssemblyRef(metadataAssemblyEmit, &libRef, std::vector<BYTE> { 0xB7, 0x7A, 0x5C, 0x56, 0x19, 0x34, 0xE0, 0x89 }, metadata, _const::mscorlib);
    }

    void GetWrapperRef(HRESULT& hr, const ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdModuleRef& libRef, const wstring& assemblyName)
    {
        ASSEMBLYMETADATA metadata{};
        metadata.usMajorVersion = 1;
        metadata.usMinorVersion = 0;
        metadata.usBuildNumber = 0;
        metadata.usRevisionNumber = 0;

        hr = CreateAssemblyRef(metadataAssemblyEmit, &libRef, std::vector<BYTE>(), metadata, assemblyName);
    }
}