#pragma once
#include <cor.h>

namespace info
{
	struct InterceptionVarInfo
	{
		mdTypeRef TypeRef;
		int LocalVarIndex;

		InterceptionVarInfo(mdTypeRef typeRef, int localVarIndex) : TypeRef(typeRef), LocalVarIndex(localVarIndex) {}
	};
}
