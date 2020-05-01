#include "AssemblyInfo.h"
#include "util/clr_const.h"

AssemblyInfo GetAssemblyInfo(ICorProfilerInfo8* info,
    AssemblyID assemblyId) {
    WCHAR assemblyName[NameMaxSize];
    DWORD len = 0;
    AppDomainID appDomainId;
    ModuleID manifestModuleId;

    auto hr = info->GetAssemblyInfo(assemblyId, NameMaxSize, &len,
        assemblyName, &appDomainId, &manifestModuleId);

    if (FAILED(hr) || len == 0) {
        return {};
    }

    WCHAR appDomainName[NameMaxSize];

    hr = info->GetAppDomainInfo(appDomainId, NameMaxSize, &len,
        appDomainName, nullptr);

    if (FAILED(hr) || len == 0) {
        return {};
    }

    return { assemblyId, assemblyName, manifestModuleId, appDomainId,
            appDomainName };
}