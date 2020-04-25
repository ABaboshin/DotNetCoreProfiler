#pragma once

#include "cor.h"

typedef struct _ImportInterception
{
	char* CallerAssembly;
	char* TargetAssemblyName;
	char* TargetMethodName;
	char* TargetTypeName;
	int TargetMethodParametersCount;
	char* InterceptorTypeName;
	char* InterceptorMethodName;
	char* InterceptorAssemblyName;
	BYTE* Signature;
	int SignatureLength;
} ImportInterception;