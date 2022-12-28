#include "profiler/CorProfiler.h"

extern "C" void __cdecl RejitAll()
{
    return instance->RejitAll();
}