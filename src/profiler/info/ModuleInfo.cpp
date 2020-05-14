#include "ModuleInfo.h"
#include "AssemblyInfo.h"
#include "const/const.h"

namespace info
{
    ModuleInfo ModuleInfo::GetModuleInfo(ICorProfilerInfo8* info, ModuleID moduleId)
    {
        std::vector<WCHAR> modulePath(_const::NameMaxSize, (WCHAR)0);
        DWORD length = 0;
        LPCBYTE baseLoadAddress;
        AssemblyID assemblyId = 0;
        DWORD moduleFlags = 0;
        const HRESULT hr = info->GetModuleInfo2(
            moduleId, &baseLoadAddress, _const::NameMaxSize, &length,
            &modulePath[0], &assemblyId, &moduleFlags);
        if (FAILED(hr) || length == 0) {
            return {};
        }
        return { moduleId, util::ToString(modulePath, length), AssemblyInfo::GetAssemblyInfo(info, assemblyId),
                moduleFlags };
    }
}