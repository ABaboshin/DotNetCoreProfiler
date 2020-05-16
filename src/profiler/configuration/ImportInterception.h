#pragma once

#include "cor.h"

namespace configuration
{
	typedef struct _ImportInterception
	{
		char* CallerAssembly;
		char* TargetAssemblyName;
		char* TargetMethodName;
		char* TargetTypeName;
		int TargetMethodParametersCount;
		char* InterceptorTypeName;
		char* InterceptorAssemblyName;
		bool IsComposed;
		char* Key;
	} ImportInterception;
}