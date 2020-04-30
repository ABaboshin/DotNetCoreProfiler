#include "CorProfiler.h"

extern "C" void __cdecl AddInterceptor(ImportInterception interception)
{
	profiler->AddInterception(interception);
}

extern "C" void __cdecl GetAssemblyBytes(BYTE** pAssemblyArray, int* assemblySize)
{
	return profiler->GetAssemblyBytes(pAssemblyArray, assemblySize);
}