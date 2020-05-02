#pragma once

#include "AssemblyInfo.h"
#include "util/util.h"

namespace info
{
    struct ModuleInfo {
        ModuleID id;
        wstring path;
        AssemblyInfo assembly;
        DWORD flags;

        ModuleInfo() : id(0), path(""_W), assembly({}), flags(0) {}
        ModuleInfo(ModuleID id, wstring path, AssemblyInfo assembly, DWORD flags)
            : id(id), path(path), assembly(assembly), flags(flags) {}

        bool IsValid() const { return id != 0; }

        bool IsWindowsRuntime() const {
            return ((flags & COR_PRF_MODULE_WINDOWS_RUNTIME) != 0);
        }

        static ModuleInfo GetModuleInfo(ICorProfilerInfo8* info, ModuleID moduleId);
    };
}