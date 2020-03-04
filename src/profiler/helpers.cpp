#include "helpers.h"
#include "AssemblyInfo.h"
#include "ModuleInfo.h"
#include "util.h"
#include <cstring>
#include <iostream>
#include <vector>

AssemblyInfo GetAssemblyInfo(ICorProfilerInfo8* info,
                             const AssemblyID& assembly_id) {
  WCHAR assembly_name[kNameMaxSize];
  DWORD assembly_name_len = 0;
  AppDomainID app_domain_id;
  ModuleID manifest_module_id;

  auto hr = info->GetAssemblyInfo(assembly_id, kNameMaxSize, &assembly_name_len,
                                  assembly_name, &app_domain_id, &manifest_module_id);

  if (FAILED(hr) || assembly_name_len == 0) {
    return {};
  }

  WCHAR app_domain_name[kNameMaxSize];
  DWORD app_domain_name_len = 0;

  hr = info->GetAppDomainInfo(app_domain_id, kNameMaxSize, &app_domain_name_len,
                              app_domain_name, nullptr);

  if (FAILED(hr) || app_domain_name_len == 0) {
    return {};
  }

  return {assembly_id, WSTRING(assembly_name), manifest_module_id, app_domain_id,
          WSTRING(app_domain_name)};
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
  return {module_id, WSTRING(module_path), GetAssemblyInfo(info, assembly_id),
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

void PrintModuleInfo(ICorProfilerInfo8* info, const ModuleID& moduleId)
{
  const ModuleInfo moduleInfo = GetModuleInfo(info, moduleId);
  std::cout << ToString(moduleInfo.assembly.name) << " " << moduleId << std::endl;
}

void PrintModuleInfo(const ModuleInfo& moduleInfo)
{
  std::cout << ToString(moduleInfo.assembly.name) << " " << moduleInfo.id << std::endl;
}
