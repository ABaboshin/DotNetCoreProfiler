#pragma once

#include "corhlpr.h"
#include "corhdr.h"
#include "cor.h"
#include "corprof.h"
#include "rewriter/ILRewriterHelper.h"
#include "RejitInfo.h"

class CorProfiler;

class MethodRewriter {
	CorProfiler* profiler;
	HRESULT DefineLocalSignature(rewriter::ILRewriter* rewriter, ModuleID moduleId, mdTypeRef exceptionTypeRef, const RejitInfo& interceptor, ULONG* exceptionIndex, ULONG* returnIndex);
	HRESULT CreateBeforeMethod(rewriter::ILRewriterHelper& helper, rewriter::ILInstr** instr, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdTypeRef interceptorTypeRef, const RejitInfo& interceptor);
	HRESULT CreateAfterMethod(rewriter::ILRewriterHelper& helper, rewriter::ILInstr** instr, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdTypeRef interceptorTypeRef, const RejitInfo& interceptor, ULONG returnIndex, mdTypeRef exceptionTypeRef, ULONG exceptionIndex);
	HRESULT InitLocalVariables(rewriter::ILRewriterHelper& helper, rewriter::ILRewriter* rewriter, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, ModuleID moduleId, const RejitInfo& interceptor, ULONG exceptionIndex, ULONG returnIndex);

	HRESULT GetTargetTypeRef(const info::TypeInfo& targetType, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdToken* targetTypeRef, bool* isValueType);
	HRESULT LogInterceptorException(rewriter::ILRewriterHelper& helper, rewriter::ILRewriter* rewriter, rewriter::ILInstr** instr, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdTypeRef exceptionTypeRef);

	HRESULT ModifyArguments(rewriter::ILRewriterHelper& helper, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdTypeRef interceptorTypeRef, const RejitInfo& interceptor);
public:
	MethodRewriter(CorProfiler* profiler) : profiler(profiler) {}

	HRESULT RewriteTargetMethod(ModuleID moduleId, mdMethodDef methodId, ICorProfilerFunctionControl* pFunctionControl);
};