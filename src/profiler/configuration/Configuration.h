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

		Configuration(std::vector<StrictInterception> strictInterceptions,
			std::vector<wstring> assemblies,
			const std::vector<AttributedInterceptor>& attributedInterceptors,
			const TypeInfo& base,
			const TypeInfo& composed) :
			StrictInterceptions(strictInterceptions),
			Assemblies(assemblies),
			AttributedInterceptors(attributedInterceptors),
			Base(base),
			Composed(composed) {}

		Configuration() {}

		static Configuration LoadConfiguration(const wstring& path);
	};
}
