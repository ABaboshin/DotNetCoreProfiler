#pragma once

#include "corhlpr.h"
#include "corhdr.h"
#include "cor.h"
#include "corprof.h"

class CorProfiler;

class MethodRewriter {
	CorProfiler* profiler;
public:
	MethodRewriter(CorProfiler* profiler) : profiler(profiler) {}

	HRESULT Rewriter(ModuleID moduleId, mdMethodDef methodId, ICorProfilerFunctionControl* pFunctionControl);
};