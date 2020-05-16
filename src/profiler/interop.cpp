#include "profiler/CorProfiler.h"

extern "C" void __cdecl GetAssemblyBytes(BYTE * *pAssemblyArray, int* assemblySize)
{
	return profilerInstance->GetAssemblyBytes(pAssemblyArray, assemblySize);
}

extern "C" void __cdecl AddInterceptor(configuration::ImportInterception interception)
{
	profilerInstance->AddInterception(interception);
}

extern "C" void __cdecl GetInterceptions(configuration::InterceptionInfo ** interceptions, char* key, int* count)
{
	profilerInstance->GetInterceptions(interceptions, ToWSTRING(key), count);
}

extern "C" void __cdecl FreeGetInterceptionsMemory(configuration::InterceptionInfo * *interceptions, int count)
{
	for (auto i = 0; i < count; i++)
	{
		delete[](*interceptions)[i].AssemblyName;
		delete[](*interceptions)[i].TypeName;
	}
}