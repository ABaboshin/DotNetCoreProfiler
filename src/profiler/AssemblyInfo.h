#pragma once

#include "cor.h"
#include "corprof.h"
#include "types.h"
#include "util.h"

struct AssemblyInfo {
  const AssemblyID id;
  const WSTRING name;
  const ModuleID manifest_module_id;
  const AppDomainID app_domain_id;
  const WSTRING app_domain_name;

  AssemblyInfo() : id(0), name(""_W), manifest_module_id(0), app_domain_id(0), app_domain_name(""_W) {}

  AssemblyInfo(AssemblyID id, WSTRING name, ModuleID manifest_module_id, AppDomainID app_domain_id,
               WSTRING app_domain_name)
      : id(id),
        name(name),
        manifest_module_id(manifest_module_id),
        app_domain_id(app_domain_id),
        app_domain_name(app_domain_name) {}

  bool IsValid() const { return id != 0; }
};
