#include "ModuleInfo.h"
#include "AssemblyInfo.h"
#include "clr_const.h"

ModuleInfo GetModuleInfo(ICorProfilerInfo8* info, ModuleID moduleId)
{
    WCHAR modulePath[NameMaxSize]{};
    DWORD len = 0;
    LPCBYTE baseLoadAddress;
    AssemblyID assemblyId = 0;
    DWORD moduleFlags = 0;
    const HRESULT hr = info->GetModuleInfo2(
        moduleId, &baseLoadAddress, NameMaxSize, &len,
        modulePath, &assemblyId, &moduleFlags);
    if (FAILED(hr) || len == 0) {
        return {};
    }
    return { moduleId, modulePath, GetAssemblyInfo(info, assemblyId),
            moduleFlags };
}