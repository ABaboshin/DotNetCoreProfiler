#pragma once

#include "cor.h"
#include "corprof.h"
#include "FunctionInfo.h"
#include "ModuleInfo.h"
#include "ComPtr.h"

void PrintModuleInfo(const ModuleInfo& moduleInfo);
void PrintModuleInfo(ICorProfilerInfo8* info, const ModuleID& moduleId);

TypeInfo GetTypeInfo(const ComPtr<IMetaDataImport2>& metadata_import,
                     const mdToken& token);

ModuleInfo GetModuleInfo(ICorProfilerInfo8* info, const ModuleID& module_id);
FunctionInfo GetFunctionInfo(const ComPtr<IMetaDataImport2>& metadata_import,
                             const mdToken& token);
