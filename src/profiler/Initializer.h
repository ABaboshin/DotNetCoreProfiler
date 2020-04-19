#pragma once

#include <vector>
#include "types.h"
#include "util.h"

class Initializer
{
public:
	WSTRING AssemblyPath;
	WSTRING AssemblyName;
	WSTRING TypeName;

	Initializer(WSTRING AssemblyPath, WSTRING AssemblyName, WSTRING TypeName) :
		AssemblyName(AssemblyName),
		TypeName(TypeName),
		AssemblyPath(AssemblyPath) {}

	Initializer() :
		AssemblyName(""_W),
		TypeName(""_W),
		AssemblyPath(""_W) {}

	Initializer& operator=(const Initializer& b)
	{
		if (this != &b)
		{
			AssemblyName = b.AssemblyName;
			TypeName = b.TypeName;
			AssemblyPath = b.AssemblyPath;
		}

		return *this;
	}

	bool IsEmpty() const {
		return AssemblyName.empty() || TypeName.empty() || AssemblyPath.empty();
	}
};

