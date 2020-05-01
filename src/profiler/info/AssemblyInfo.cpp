#include "AssemblyInfo.h"
#include "const/const.h"

AssemblyInfo GetAssemblyInfo(ICorProfilerInfo8* info,
    AssemblyID assemblyId) {
    WCHAR assemblyName[_const::NameMaxSize];
    DWORD len = 0;
    AppDomainID appDomainId;
    ModuleID manifestModuleId;

    auto hr = info->GetAssemblyInfo(assemblyId, _const::NameMaxSize, &len,
        assemblyName, &appDomainId, &manifestModuleId);

    if (FAILED(hr) || len == 0) {
        return {};
    }

    WCHAR appDomainName[_const::NameMaxSize];

    hr = info->GetAppDomainInfo(appDomainId, _const::NameMaxSize, &len,
        appDomainName, nullptr);

    if (FAILED(hr) || len == 0) {
        return {};
    }

    return { assemblyId, assemblyName, manifestModuleId, appDomainId,
            appDomainName };
}