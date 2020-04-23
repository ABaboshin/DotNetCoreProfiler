#pragma once

#include "cor.h"
#include "corprof.h"
#include "util.h"

struct AssemblyInfo {
  AssemblyID id;
  wstring name;
  ModuleID manifestModuleId;
  AppDomainID appDomainId;
  wstring appDomainName;

  AssemblyInfo() : id(0), name(""_W), manifestModuleId(0), appDomainId(0), appDomainName(""_W) {}

  AssemblyInfo(AssemblyID id, wstring name, ModuleID manifest_module_id, AppDomainID app_domain_id,
      wstring app_domain_name)
      : id(id),
        name(name),
        manifestModuleId(manifest_module_id),
        appDomainId(app_domain_id),
        appDomainName(app_domain_name) {}

  bool IsValid() const { return id != 0; }
};
