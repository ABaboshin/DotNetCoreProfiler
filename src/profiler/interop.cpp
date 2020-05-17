#include "profiler/CorProfiler.h"

extern "C" void __cdecl GetAssemblyBytes(BYTE * *pAssemblyArray, int* assemblySize)
{
	return profilerInstance->GetAssemblyBytes(pAssemblyArray, assemblySize);
}
