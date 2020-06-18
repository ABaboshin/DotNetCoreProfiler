#include <iostream>
#include <fstream>
#include <utility>
#include "nlohmann/json.hpp"
#include "configuration/Configuration.h"

namespace configuration
{
	Configuration LoadFromStream(std::ifstream& stream);
	std::pair<StrictInterception, bool> LoadInterceptionFromJson(const nlohmann::json::value_type& src);
	std::pair<TypeInfo, bool> LoadTypeInfoFromJson(const nlohmann::json::value_type& src);
	std::pair<TargetMethod, bool> LoadTargetFromJson(const nlohmann::json::value_type& src);
	std::pair<AttributedInterceptor, bool> LoadAttributedInterceptorFromJson(const nlohmann::json::value_type& src);
	std::pair<TypeInfo, bool> LoadTypeInfoFromJson(const nlohmann::json::value_type& src);
	std::pair<MethodFinder, bool> LoadMethodFinder(const nlohmann::json::value_type& src);

	Configuration Configuration::LoadConfiguration(const std::string& path)
	{
		Configuration configuration;

		try
		{
			std::ifstream stream;
			stream.open(path);

			if (static_cast<bool>(stream)) {
				return LoadFromStream(stream);
			}
		}
		catch (...)
		{
			auto ex = std::current_exception();
			if (ex) {
				std::rethrow_exception(ex);
			}
		}

		return configuration;
	}

	Configuration LoadFromStream(std::ifstream& stream)
	{
		std::vector<StrictInterception> interceptions{};
		std::vector<wstring> assemblies{};
		std::unordered_set<wstring> skipAssemblies{};
		std::unordered_map<wstring, AttributedInterceptor> attributedInterceptors{};
		std::vector<MethodFinder> methodFinders{};

		nlohmann::json j;
		stream >> j;

		for (auto& el : j["strict"]) {
			auto i = LoadInterceptionFromJson(el);
			if (std::get<1>(i)) {
				interceptions.push_back(std::get<0>(i));
			}
		}

		for (auto& el : j["methodFinders"]) {
			auto i = LoadMethodFinder(el);
			if (std::get<1>(i)) {
				methodFinders.push_back(std::get<0>(i));
			}
		}

		for (auto& el : j["assemblies"]) {
			assemblies.push_back(ToWSTRING(el));
		}

		for (auto& el : j["skipAssemblies"]) {
			skipAssemblies.insert(ToWSTRING(el));
		}

		for (auto& el : j["attributed"]) {
			auto i = LoadAttributedInterceptorFromJson(el);
			if (std::get<1>(i)) {
				attributedInterceptors.insert(std::make_pair(std::get<0>(i).AttributeType, std::get<0>(i)));
			}
		}

		auto interceptorInterface = LoadTypeInfoFromJson(j["interceptorInterface"]);
		auto composedInterceptor = LoadTypeInfoFromJson(j["composedInterceptor"]);
		auto methodFinderInterface = LoadTypeInfoFromJson(j["methodFinderInterface"]);

		return {
			interceptions,
			assemblies,
			attributedInterceptors,
			std::get<0>(interceptorInterface),
			std::get<0>(composedInterceptor),
			std::get<0>(methodFinderInterface),
			methodFinders,
			skipAssemblies
		};
	}

	std::pair<TypeInfo, bool> LoadTypeInfoFromJson(const nlohmann::json::value_type& src)
	{
		if (!src.is_object()) {
			return std::make_pair<TypeInfo, bool>({}, false);
		}

		auto assemblyName = ToWSTRING(src.value("AssemblyName", ""));
		auto typeName = ToWSTRING(src.value("TypeName", ""));

		return std::make_pair<TypeInfo, bool>({ assemblyName , typeName }, true);
	}

	std::pair<AttributedInterceptor, bool> LoadAttributedInterceptorFromJson(const nlohmann::json::value_type& src)
	{
		if (!src.is_object()) {
			return std::make_pair<AttributedInterceptor, bool>({}, false);
		}

		auto attributeType = ToWSTRING(src.value("AttributeType", ""));
		auto interceptor = std::get<0>(LoadTypeInfoFromJson(src["Interceptor"]));

		return std::make_pair<AttributedInterceptor, bool>({ interceptor, attributeType }, true);
	}

	std::pair<MethodFinder, bool> LoadMethodFinder(const nlohmann::json::value_type& src)
	{
		if (!src.is_object()) {
			return std::make_pair<MethodFinder, bool>({}, false);
		}

		auto typeInfo = std::get<0>(LoadTypeInfoFromJson(src["MethodFinder"]));
		auto target = std::get<0>(LoadTargetFromJson(src["Target"]));

		return std::make_pair<MethodFinder, bool>({ target, typeInfo }, true);
	}

	std::pair<StrictInterception, bool> LoadInterceptionFromJson(const nlohmann::json::value_type& src)
	{
		if (!src.is_object()) {
			return std::make_pair<StrictInterception, bool>({}, false);
		}

		auto interceptor = std::get<0>(LoadTypeInfoFromJson(src["Interceptor"]));
		auto target = std::get<0>(LoadTargetFromJson(src["Target"]));
		std::unordered_set<wstring> ignoreCallerAssemblies{};

		for (auto& el : src["IgnoreCallerAssemblies"]) {
			ignoreCallerAssemblies.insert(ToWSTRING(el));
		}

		return std::make_pair<StrictInterception, bool>({ ignoreCallerAssemblies, target, interceptor }, true);
	}

	std::pair<TargetMethod, bool> LoadTargetFromJson(const nlohmann::json::value_type& src)
	{
		if (!src.is_object()) {
			return std::make_pair<TargetMethod, bool>({}, false);
		}

		auto assemblyName = ToWSTRING(src.value("AssemblyName", ""));
		auto typeName = ToWSTRING(src.value("TypeName", ""));
		auto methodName = ToWSTRING(src.value("MethodName", ""));
		auto methodParametersCount = src.value("MethodParametersCount", 0);

		return std::make_pair<TargetMethod, bool>({ assemblyName, typeName , methodName , methodParametersCount }, true);
	}
}