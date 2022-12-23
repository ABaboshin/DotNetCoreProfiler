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
	HRESULT CreateBeforeMethod(rewriter::ILRewriterHelper& helper, rewriter::ILInstr** instr, util::ComPtr< IMetaDataEmit2> metadataEmit, mdTypeRef interceptorTypeRef, const RejitInfo& interceptor);
	HRESULT CreateAfterMethod(rewriter::ILRewriterHelper& helper, rewriter::ILInstr** instr, util::ComPtr< IMetaDataEmit2> metadataEmit, mdTypeRef interceptorTypeRef, const RejitInfo& interceptor);
public:
	MethodRewriter(CorProfiler* profiler) : profiler(profiler) {}

	HRESULT Rewriter(ModuleID moduleId, mdMethodDef methodId, ICorProfilerFunctionControl* pFunctionControl);
	
};