#pragma once

#include <vector>
#include "configuration/AttributedInterceptor.h"
#include "configuration/TypeInfo.h"
#include "configuration/StrictInterception.h"


namespace configuration
{
	struct Configuration
	{
		std::vector<StrictInterception> StrictInterceptions{};
		std::vector<wstring> Assemblies;
		std::vector<AttributedInterceptor> AttributedInterceptors;
		TypeInfo Base{};
		TypeInfo Composed{};
		std::vector<wstring> SkipAssemblies;

		Configuration(std::vector<StrictInterception> strictInterceptions,
			std::vector<wstring>& assemblies,
			const std::vector<AttributedInterceptor>& attributedInterceptors,
			const TypeInfo& base,
			const TypeInfo& composed,
			std::vector<wstring>& skipAssemblies) :
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
