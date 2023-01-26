#include <iostream>
#include <vector>
#include "util/util.h"
#include "profiler/CorProfiler.h"

struct _TargetMethodInfo {
    WCHAR* assemblyName;
    WCHAR* typeName;
    WCHAR* methodName;
    int MethodParametersCount;
};

EXTERN_C VOID STDAPICALLTYPE AddMethodParameters(_TargetMethodInfo mi, WCHAR** parameters, int parametersLength)
{
    std::vector<util::wstring> params;
    for (size_t i = 0; i < parametersLength; i++)
    {
        params.push_back(util::wstring(parameters[i]));
    }

    instance->AddMethodParameters(util::wstring(mi.assemblyName), util::wstring(mi.typeName), util::wstring(mi.methodName), mi.MethodParametersCount, params);
}

EXTERN_C VOID STDAPICALLTYPE AddMethodVariables(_TargetMethodInfo mi, WCHAR** variables, int variablesLength)
{
    std::vector<util::wstring> vars;
    for (size_t i = 0; i < variablesLength; i++)
    {
        vars.push_back(util::wstring(variables[i]));
    }

    instance->AddMethodVariables(util::wstring(mi.assemblyName), util::wstring(mi.typeName), util::wstring(mi.methodName), mi.MethodParametersCount, vars);
}

EXTERN_C VOID STDAPICALLTYPE AddDebuggerOffset(_TargetMethodInfo mi, int offset)
{
    instance->AddDebuggerOffset(util::wstring(mi.assemblyName), util::wstring(mi.typeName), util::wstring(mi.methodName), mi.MethodParametersCount, offset);
}

EXTERN_C VOID STDAPICALLTYPE StartDebugger()
{
    instance->StartDebugger();
}