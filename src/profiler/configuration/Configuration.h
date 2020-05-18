#pragma once

#include <vector>
#include "configuration/AttributedInterceptor.h"
#include "configuration/BaseClass.h"
#include "configuration/StrictInterception.h"


namespace configuration
{
	struct Configuration
	{
		std::vector<StrictInterception> StrictInterceptions{};
		std::vector<wstring> Assemblies;
		std::vector<AttributedInterceptor> AttributedInterceptors;
		BaseClass Base{};

		Configuration(std::vector<StrictInterception> strictInterceptions, std::vector<wstring> assemblies, const std::vector<AttributedInterceptor>& attributedInterceptors, const BaseClass& base) :
			StrictInterceptions(strictInterceptions), Assemblies(assemblies), AttributedInterceptors(attributedInterceptors), Base(base) {}

		Configuration() {}

		static Configuration LoadConfiguration(const wstring& path);
	};
}
