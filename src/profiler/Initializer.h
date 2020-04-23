#pragma once

#include <vector>
#include "util.h"

class Initializer
{
public:
	wstring AssemblyPath;
	wstring AssemblyName;
	wstring TypeName;

	Initializer(const wstring& assemblyPath, const wstring& assemblyName, const wstring& typeName) :
		AssemblyName(assemblyName),
		TypeName(typeName),
		AssemblyPath(assemblyPath) {}

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

