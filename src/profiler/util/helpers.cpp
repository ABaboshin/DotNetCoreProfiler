#include <cstring>
#include <iostream>
#include <vector>

#include "info/AssemblyInfo.h"
#include "info/ModuleInfo.h"
#include "const/const.h"
#include "helpers.h"
#include "util.h"
#include "logging/logging.h"

namespace util
{
    HRESULT CreateAssemblyRef(const ComPtr<IMetaDataAssemblyEmit> metadataAssemblyEmit, mdAssemblyRef* libRef, const std::vector<BYTE>& public_key, ASSEMBLYMETADATA metadata, const wstring& assemblyName) {
        HRESULT hr = metadataAssemblyEmit->DefineAssemblyRef(
            (void*)public_key.data(),
            (ULONG)public_key.size(),
            assemblyName.c_str(), &metadata, NULL, 0, 0,
            libRef);

        return hr;
    }

    HRESULT GetMsCorLibRef(const ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdModuleRef& libRef)
    {
        ASSEMBLYMETADATA metadata{};
        metadata.usMajorVersion = 4;
        metadata.usMinorVersion = 0;
        metadata.usBuildNumber = 0;
        metadata.usRevisionNumber = 0;

        return CreateAssemblyRef(metadataAssemblyEmit, &libRef, std::vector<BYTE> { 0xB7, 0x7A, 0x5C, 0x56, 0x19, 0x34, 0xE0, 0x89 }, metadata, _const::mscorlib);
    }

    HRESULT GetAssemblyRef(const ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdModuleRef& libRef, const wstring& assemblyName)
    {
        ASSEMBLYMETADATA metadata{};
        metadata.usMajorVersion = 1;
        metadata.usMinorVersion = 0;
        metadata.usBuildNumber = 0;
        metadata.usRevisionNumber = 0;

        return CreateAssemblyRef(metadataAssemblyEmit, &libRef, std::vector<BYTE>(), metadata, assemblyName);
    }
}