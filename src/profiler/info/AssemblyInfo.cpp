#include "AssemblyInfo.h"
#include "const/const.h"

namespace info
{
    AssemblyInfo AssemblyInfo::GetAssemblyInfo(ICorProfilerInfo8* info,
        AssemblyID assemblyId) {
        std::vector<WCHAR> assemblyName(_const::NameMaxSize, (WCHAR)0);
        DWORD length = 0;
        AppDomainID appDomainId;
        ModuleID manifestModuleId;

        auto hr = info->GetAssemblyInfo(assemblyId, _const::NameMaxSize, &length,
            &assemblyName[0], &appDomainId, &manifestModuleId);

        if (FAILED(hr) || length == 0) {
            return {};
        }

        WCHAR appDomainName[_const::NameMaxSize];

        hr = info->GetAppDomainInfo(appDomainId, _const::NameMaxSize, &length,
            appDomainName, nullptr);

        if (FAILED(hr) || length == 0) {
            return {};
        }

        return { assemblyId, util::ToString(assemblyName, length), manifestModuleId, appDomainId,
                appDomainName };
    }
}