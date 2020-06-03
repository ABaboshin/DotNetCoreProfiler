#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "configuration/AttributedInterceptor.h"
#include "configuration/TypeInfo.h"
#include "configuration/StrictInterception.h"


namespace configuration
{
	struct Configuration
	{
		std::vector<StrictInterception> StrictInterceptions{};
		std::vector<wstring> Assemblies;
		std::unordered_map<wstring, AttributedInterceptor> AttributedInterceptors;
		TypeInfo Base{};
		TypeInfo Composed{};
		std::unordered_set<wstring> SkipAssemblies;

		Configuration(std::vector<StrictInterception> strictInterceptions,
			std::vector<wstring>& assemblies,
			const std::unordered_map<wstring, AttributedInterceptor>& attributedInterceptors,
			const TypeInfo& base,
			const TypeInfo& composed,
			std::unordered_set<wstring>& skipAssemblies) :
			StrictInterceptions(strictInterceptions),
			Assemblies(assemblies),
			AttributedInterceptors(attributedInterceptors),
			Base(base),
			Composed(composed),
			SkipAssemblies(skipAssemblies) {}

		Configuration() {}

		static Configuration LoadConfiguration(const wstring& path);
	};
}
