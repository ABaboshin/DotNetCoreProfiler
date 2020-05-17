#pragma once

#include <vector>
#include "configuration/StrictInterception.h"
#include "configuration/AttributedInterceptor.h"
#include "configuration/AssemblyInizializer.h"

namespace configuration
{
	struct Configuration
	{
		std::vector<StrictInterception> StrictInterceptions{};
		std::vector<AssemblyInitializer> Assemblies;
		std::vector<AttributedInterceptor> AttributedInterceptors;

		Configuration(std::vector<StrictInterception> strictInterceptions, std::vector<AssemblyInitializer> assemblies, const std::vector<AttributedInterceptor>& attributedInterceptors) :
			StrictInterceptions(strictInterceptions), Assemblies(assemblies), AttributedInterceptors(attributedInterceptors) {}

		Configuration() {}

		static Configuration LoadConfiguration(const wstring& path);
	};
}
