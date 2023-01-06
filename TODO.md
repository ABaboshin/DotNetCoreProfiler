Masstransit 7/8
Debugger:
  - try/catch
  - update all related leave_s to debugger.start
Strict/Trace
  - use dedicated try/catch for each
Add traces on the fly via interop
Add debug on the fly via interop
CreateILRewriter unique ptr
remove code duplicates => extract it into helper methods https://pmd.github.io/latest/pmd_userdocs_cpd.html
update lib/coreclr and use the same build toolchain as dotnet/runtime does https://github.com/dotnet/runtime/blob/main/docs/workflow/building/coreclr/linux-instructions.md#docker-images
add profiler time logs (jitcompilationstarted, moduleloadedfinished, inject debugger, rejitparameters)
check if before/after exist for the strict interceptors
cor_signature -> vector?
pointer -> reference in c++ method params
cache references to known types
ngen
skip assemblies vs enabled assemblies
