#pragma once

#include "cor.h"
#include "corprof.h"
#include "util/util.h"
#include "util/ComPtr.h"

namespace info
{
    using namespace util;

    struct AssemblyInfo {
        AssemblyID id;
        wstring name;
        ModuleID manifestModuleId;
        AppDomainID appDomainId;
        wstring appDomainName;

        AssemblyInfo() : id(0), name(""_W), manifestModuleId(0), appDomainId(0), appDomainName(""_W) {}

        AssemblyInfo(AssemblyID id, wstring name, ModuleID manifestModuleId, AppDomainID appDomainId,
            wstring appDomainName)
            : id(id),
            name(name),
            manifestModuleId(manifestModuleId),
            appDomainId(appDomainId),
            appDomainName(appDomainName) {}

        bool IsValid() const { return id != 0; }
    };

    AssemblyInfo GetAssemblyInfo(ICorProfilerInfo8* info, AssemblyID assemblyId);
}