#pragma once

#include <vector>
#include "configuration/StrictInterception.h"
#include "configuration/AttributedInterceptor.h"

namespace configuration
{
	struct Configuration
	{
		std::vector<StrictInterception> StrictInterceptions{};
		std::vector<wstring> Assemblies;
		std::vector<AttributedInterceptor> AttributedInterceptors;

		Configuration(std::vector<StrictInterception> strictInterceptions, std::vector<wstring> assemblies, const std::vector<AttributedInterceptor>& attributedInterceptors) :
			StrictInterceptions(strictInterceptions), Assemblies(assemblies), AttributedInterceptors(attributedInterceptors) {}

		Configuration() {}

		static Configuration LoadConfiguration(const wstring& path);
	};
}
