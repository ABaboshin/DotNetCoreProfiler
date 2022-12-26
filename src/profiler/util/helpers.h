#pragma once

#include "cor.h"
#include "corprof.h"
#include "info/FunctionInfo.h"
#include "info/ModuleInfo.h"
#include "ComPtr.h"

namespace util
{
	HRESULT GetMsCorLibRef(const ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdModuleRef& libRef);
	HRESULT GetAssemblyRef(const ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdModuleRef& libRef, const wstring& assemblyName);
}
