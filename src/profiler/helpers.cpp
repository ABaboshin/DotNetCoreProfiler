#include "helpers.h"
#include "AssemblyInfo.h"
#include "ModuleInfo.h"
#include "util.h"
#include <cstring>
#include <iostream>
#include <vector>
#include "clr_const.h"

AssemblyInfo GetAssemblyInfo(ICorProfilerInfo8* info,
                             const AssemblyID& assembly_id) {
  WCHAR assembly_name[NameMaxSize];
  DWORD assembly_name_len = 0;
  AppDomainID app_domain_id;
  ModuleID manifest_module_id;

  auto hr = info->GetAssemblyInfo(assembly_id, NameMaxSize, &assembly_name_len,
                                  assembly_name, &app_domain_id, &manifest_module_id);

  if (FAILED(hr) || assembly_name_len == 0) {
    return {};
  }

  WCHAR app_domain_name[NameMaxSize];
  DWORD app_domain_name_len = 0;

  hr = info->GetAppDomainInfo(app_domain_id, NameMaxSize, &app_domain_name_len,
                              app_domain_name, nullptr);

  if (FAILED(hr) || app_domain_name_len == 0) {
    return {};
  }

  return {assembly_id, assembly_name, manifest_module_id, app_domain_id,
          app_domain_name};
}

ModuleInfo GetModuleInfo(ICorProfilerInfo8* info, const ModuleID& module_id)
{
  const DWORD module_path_size = 260;
  WCHAR module_path[module_path_size]{};
  DWORD module_path_len = 0;
  LPCBYTE base_load_address;
  AssemblyID assembly_id = 0;
  DWORD module_flags = 0;
  const HRESULT hr = info->GetModuleInfo2(
      module_id, &base_load_address, module_path_size, &module_path_len,
      module_path, &assembly_id, &module_flags);
  if (FAILED(hr) || module_path_len == 0) {
    return {};
  }
  return {module_id, module_path, GetAssemblyInfo(info, assembly_id),
          module_flags};
}

std::vector<BYTE> GetSignatureByteRepresentation(
    ULONG signature_length, PCCOR_SIGNATURE raw_signature) {
  std::vector<BYTE> signature_data(signature_length);
  for (ULONG i = 0; i < signature_length; i++) {
    signature_data[i] = raw_signature[i];
  }

  return signature_data;
}

void GetMsCorLibRef(HRESULT& hr, const ComPtr<IMetaDataAssemblyEmit>& pMetadataAssemblyEmit, mdModuleRef& libRef)
{
    ASSEMBLYMETADATA metadata{};
    metadata.usMajorVersion = 4;
    metadata.usMinorVersion = 0;
    metadata.usBuildNumber = 0;
    metadata.usRevisionNumber = 0;

    hr = CreateAssemblyRef(pMetadataAssemblyEmit, &libRef, std::vector<BYTE> { 0xB7, 0x7A, 0x5C, 0x56, 0x19, 0x34, 0xE0, 0x89 }, metadata, mscorlib);
}

void GetWrapperRef(HRESULT& hr, const ComPtr<IMetaDataAssemblyEmit>& pMetadataAssemblyEmit, mdModuleRef& libRef, const wstring& assemblyName)
{
    ASSEMBLYMETADATA metadata{};
    metadata.usMajorVersion = 1;
    metadata.usMinorVersion = 0;
    metadata.usBuildNumber = 0;
    metadata.usRevisionNumber = 0;

    hr = CreateAssemblyRef(pMetadataAssemblyEmit, &libRef, std::vector<BYTE>(), metadata, assemblyName);
}

HRESULT CreateAssemblyRef(const ComPtr<IMetaDataAssemblyEmit> pMetadataAssemblyEmit, mdAssemblyRef* mscorlib_ref, const std::vector<BYTE>& public_key, ASSEMBLYMETADATA metadata, const wstring& assemblyName) {
    HRESULT hr = pMetadataAssemblyEmit->DefineAssemblyRef(
        (void*)public_key.data(),
        (ULONG)public_key.size(),
        assemblyName.c_str(), &metadata, NULL, 0, 0,
        mscorlib_ref);

    return hr;
}