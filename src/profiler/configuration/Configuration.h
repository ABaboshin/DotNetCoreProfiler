#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "AttributedInterceptor.h"
#include "TypeInfo.h"
#include "StrictInterception.h"
#include "MethodFinder.h"


namespace configuration
{
	struct Configuration
	{
		std::vector<StrictInterception> StrictInterceptions{};
		std::vector<wstring> Assemblies;
		std::unordered_map<wstring, AttributedInterceptor> AttributedInterceptors;
		TypeInfo InterceptorInterface{};
		TypeInfo ComposedInterceptor{};
		TypeInfo MethodFinderInterface{};
		std::vector<MethodFinder> MethodFinders{};
		std::unordered_set<wstring> SkipAssemblies;
		wstring Loader;
		//std::unordered_set<wstring> EnabledAssemblies;

		Configuration(std::vector<StrictInterception> strictInterceptions,
			std::vector<wstring>& assemblies,
			const std::unordered_map<wstring, AttributedInterceptor>& attributedInterceptors,
			const TypeInfo& base,
			const TypeInfo& composed,
			const TypeInfo& methodFinderInterface,
			std::vector<MethodFinder> methodFinders,
			std::unordered_set<wstring>& skipAssemblies,
			wstring loader
			//std::unordered_set<wstring>& enabledAssemblies
		) :
			StrictInterceptions(strictInterceptions),
			Assemblies(assemblies),
			AttributedInterceptors(attributedInterceptors),
			InterceptorInterface(base),
			ComposedInterceptor(composed),
			MethodFinderInterface(methodFinderInterface),
			MethodFinders(methodFinders),
			SkipAssemblies(skipAssemblies),
			Loader(loader)
			//EnabledAssemblies(enabledAssemblies)
		{}

		Configuration() {}

		static Configuration LoadFromStream(std::istream& stream);
		static Configuration LoadConfiguration(const std::string& path);
	};
}
