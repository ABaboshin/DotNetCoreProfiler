#include "AssemblyInfo.h"
#include "const/const.h"

namespace info
{
    AssemblyInfo AssemblyInfo::GetAssemblyInfo(ICorProfilerInfo8* info,
        AssemblyID assemblyId) {
        std::vector<WCHAR> assemblyName(MAX_CLASS_NAME, (WCHAR)0);
        DWORD assemblyNameLength = 0;
        AppDomainID appDomainId;
        ModuleID manifestModuleId;

        auto hr = info->GetAssemblyInfo(assemblyId, MAX_CLASS_NAME, &assemblyNameLength,
            &assemblyName[0], &appDomainId, &manifestModuleId);

        if (FAILED(hr) || assemblyNameLength == 0) {
            return {};
        }

        std::vector<WCHAR> appDomainName(MAX_CLASS_NAME, (WCHAR)0);
        DWORD appDomainNameLength = 0;

        hr = info->GetAppDomainInfo(appDomainId, MAX_CLASS_NAME, &appDomainNameLength,
            &appDomainName[0], nullptr);

        if (FAILED(hr) || appDomainNameLength == 0) {
            return {};
        }

        return { assemblyId, util::ToString(assemblyName, assemblyNameLength), manifestModuleId, appDomainId,
                util::ToString(appDomainName, appDomainNameLength) };
    }
}