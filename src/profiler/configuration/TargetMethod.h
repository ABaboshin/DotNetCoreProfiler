#pragma once

#include "util/util.h"

namespace configuration
{
	using namespace util;

	struct TargetMethod {
		wstring AssemblyName;
		wstring TypeName;
		wstring MethodName;
		int MethodParametersCount;

		TargetMethod(const wstring& assemblyName, const wstring& typeName, const wstring& methodName, int methodParametersCount) :
			AssemblyName(assemblyName),
			TypeName(typeName),
			MethodName(methodName),
			MethodParametersCount(methodParametersCount) {}

		TargetMethod() :
			AssemblyName(""_W),
			TypeName(""_W),
			MethodName(""_W),
			MethodParametersCount(0) {}

		bool operator==(const TargetMethod& other) const
		{
			return (AssemblyName == other.AssemblyName
				&& TypeName == other.TypeName
				&& MethodName == other.MethodName
				&& MethodParametersCount == other.MethodParametersCount);
		}
	};
}

namespace std {

	template <>
	struct hash<configuration::TargetMethod>
	{
		std::size_t operator()(const configuration::TargetMethod& k) const
		{
			using std::size_t;
			using std::hash;
			using std::string;

			// Compute individual hash values for first,
			// second and third and combine them using XOR
			// and bit shifting:

			return ((std::hash<util::wstring>()(k.AssemblyName)
				^ (std::hash<util::wstring>()(k.TypeName) << 1)) >> 1)
				^ ((std::hash<util::wstring>()(k.MethodName) << 1)
				^ (std::hash<int>()(k.MethodParametersCount) << 1)) >> 1;
		}
	};

}