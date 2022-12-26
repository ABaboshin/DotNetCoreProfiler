#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "LoaderInfo.h"
#include "TypeInfo.h"
#include "StrictInterception.h"
#include "DefaultInitializerInfo.h"
#include "ExceptionLoggerInfo.h"


namespace configuration
{
	struct Configuration
	{
		std::vector<StrictInterception> StrictInterceptions{};
		std::vector<wstring> Assemblies;
		std::unordered_set<wstring> SkipAssemblies;
		LoaderInfo Loader;
		DefaultInitializerInfo DefaultInitializer;
		ExceptionLoggerInfo ExceptionLogger;

		Configuration(std::vector<StrictInterception> strictInterceptions,
			std::vector<wstring>& assemblies,
			std::unordered_set<wstring>& skipAssemblies,
			LoaderInfo loader,
			DefaultInitializerInfo defaultInitializer,
			ExceptionLoggerInfo exceptionLogger
		) :
			StrictInterceptions(strictInterceptions),
			Assemblies(assemblies),
			SkipAssemblies(skipAssemblies),
			Loader(loader),
			DefaultInitializer(defaultInitializer),
			ExceptionLogger(exceptionLogger)
		{}

		Configuration() {}

		static Configuration LoadFromStream(std::istream& stream);
		static Configuration LoadConfiguration(const std::string& path);
	};
}
