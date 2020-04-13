#pragma once

#include "AssemblyInfo.h"
#include "util.h"

struct ModuleInfo {
  const ModuleID id;
  const WSTRING path;
  const AssemblyInfo assembly;
  const DWORD flags;

  ModuleInfo() : id(0), path(""_W), assembly({}), flags(0) {}
  ModuleInfo(ModuleID id, WSTRING path, AssemblyInfo assembly, DWORD flags)
      : id(id), path(path), assembly(assembly), flags(flags) {}

  bool IsValid() const { return id != 0; }

  bool IsWindowsRuntime() const {
    return ((flags & COR_PRF_MODULE_WINDOWS_RUNTIME) != 0);
  }
};
