#pragma once

#include "cor.h"
#include "corprof.h"
#include "FunctionInfo.h"
#include "ModuleInfo.h"
#include "ComPtr.h"

TypeInfo GetTypeInfo(const ComPtr<IMetaDataImport2>& metadata_import,
                     const mdToken& token);

ModuleInfo GetModuleInfo(ICorProfilerInfo8* info, const ModuleID& module_id);
FunctionInfo GetFunctionInfo(const ComPtr<IMetaDataImport2>& metadata_import,
                             const mdToken& token);

void GetMsCorLibRef(HRESULT& hr, const ComPtr<IMetaDataAssemblyEmit>& pMetadataAssemblyEmit, mdModuleRef& libRef);
void GetWrapperRef(HRESULT& hr, const ComPtr<IMetaDataAssemblyEmit>& pMetadataAssemblyEmit, mdModuleRef& libRef, const wstring& assemblyName);

HRESULT CreateAssemblyRef(const ComPtr< IMetaDataAssemblyEmit> pMetadataAssemblyEmit, mdAssemblyRef* mscorlib_ref, const std::vector<BYTE>& public_key, ASSEMBLYMETADATA metadata, const wstring& assemblyName);