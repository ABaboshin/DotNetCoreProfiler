#include "profiler/CorProfiler.h"

extern "C" void __cdecl AddInterceptor(configuration::ImportInterception interception)
{
	profiler::profiler->AddInterception(interception);
}

extern "C" void __cdecl GetAssemblyBytes(BYTE** pAssemblyArray, int* assemblySize)
{
	return profiler::profiler->GetAssemblyBytes(pAssemblyArray, assemblySize);
}