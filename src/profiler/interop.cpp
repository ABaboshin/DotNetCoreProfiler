#include "CorProfiler.h"

EXTERN_C BOOL STDAPICALLTYPE IsProfilerAttached() {
  return profiler->IsAttached();
}

