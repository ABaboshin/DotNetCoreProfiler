#pragma once

#include "cor.h"
#include "corprof.h"
#include "FunctionInfo.h"
#include "ModuleInfo.h"
#include "ComPtr.h"

wstring GetSigTypeTokName(PCCOR_SIGNATURE& pbCur, const ComPtr<IMetaDataImport2>& metadaImport);

void GetMsCorLibRef(HRESULT& hr, const ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdModuleRef& libRef);
void GetWrapperRef(HRESULT& hr, const ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdModuleRef& libRef, const wstring& assemblyName);
