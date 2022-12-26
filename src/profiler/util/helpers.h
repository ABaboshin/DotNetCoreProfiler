#pragma once

#include "cor.h"
#include "corprof.h"
#include "info/FunctionInfo.h"
#include "info/ModuleInfo.h"
#include "ComPtr.h"

namespace util
{
	HRESULT GetMsCorLibRef(const ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdModuleRef& libRef);
	HRESULT GetWrapperRef(const ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdModuleRef& libRef, const wstring& assemblyName);
	//mdToken GetTypeToken(ComPtr<IMetaDataEmit2>& metadataEmit, mdAssemblyRef mscorlibRef, std::vector<BYTE>& type);

	HRESULT GetObjectTypeRef(util::ComPtr< IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdToken* objectTypeRef);
}
