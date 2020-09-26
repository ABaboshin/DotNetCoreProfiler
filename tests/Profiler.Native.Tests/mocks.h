#pragma once

#include "cor.h"

static void RaiseException(DWORD dwExceptionCode,
	DWORD dwExceptionFlags,
	DWORD nNumberOfArguments,
	CONST ULONG_PTR* lpArguments);