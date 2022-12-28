#include <iostream>
#include <fstream>
#include <utility>
#include "nlohmann/json.hpp"
#include "configuration/Configuration.h"

namespace configuration
{
	std::pair<StrictInterception, bool> LoadInterceptionFromJson(const nlohmann::json::value_type& src);
	std::pair<TypeInfo, bool> LoadTypeInfoFromJson(const nlohmann::json::value_type& src);
	std::pair<TargetMethod, bool> LoadTargetFromJson(const nlohmann::json::value_type& src);
	std::pair<TypeInfo, bool> LoadTypeInfoFromJson(const nlohmann::json::value_type& src);
	DefaultInitializerInfo LoadDefaultInitializerFromJson(const nlohmann::json::value_type& src);
	ExceptionLoggerInfo LoadExceptionLoggerFromJson(const nlohmann::json::value_type& src);

	LoaderInfo LoadLoaderFromJson(const nlohmann::json::value_type& src);

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

	Configuration Configuration::LoadFromStream(std::istream& stream)
	{
		std::vector<StrictInterception> interceptions{};
		std::unordered_set<wstring> skipAssemblies{};

		nlohmann::json j;
		stream >> j;

		auto loader = LoadLoaderFromJson(j["Loader"]);
		auto defaultInitializer = LoadDefaultInitializerFromJson(j["DefaultInitializer"]);
		auto exceptionLogger = LoadExceptionLoggerFromJson(j["ExceptionLogger"]);

		for (auto& el : j["Strict"]) {
			auto i = LoadInterceptionFromJson(el);
			if (std::get<1>(i)) {
				interceptions.push_back(std::get<0>(i));
			}
		}

		for (auto& el : j["SkipAssemblies"]) {
			skipAssemblies.insert(ToWSTRING(el));
		}

		return {
			interceptions,
			skipAssemblies,
			loader,
			defaultInitializer,
			exceptionLogger
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

	std::pair<StrictInterception, bool> LoadInterceptionFromJson(const nlohmann::json::value_type& src)
	{
		if (!src.is_object()) {
			return std::make_pair<StrictInterception, bool>({}, false);
		}

		auto interceptor = std::get<0>(LoadTypeInfoFromJson(src["Interceptor"]));
		auto target = std::get<0>(LoadTargetFromJson(src["Target"]));

		return std::make_pair<StrictInterception, bool>({ target, interceptor}, true);
	}

	LoaderInfo LoadLoaderFromJson(const nlohmann::json::value_type& src)
	{
		if (!src.is_object()) {
			throw std::string("Loader not found");
		}

		return { ToWSTRING(src.value("AssemblyPath", "")) , ToWSTRING(src.value("TypeName", "")) };
	}

	DefaultInitializerInfo LoadDefaultInitializerFromJson(const nlohmann::json::value_type& src)
	{
		if (!src.is_object()) {
			throw std::string("DefaultInitializer not found");
		}

		return { ToWSTRING(src.value("AssemblyPath", "")) , ToWSTRING(src.value("TypeName", "")), ToWSTRING(src.value("MethodName", "")), ToWSTRING(src.value("AssemblyName", "")) };
	}

	ExceptionLoggerInfo LoadExceptionLoggerFromJson(const nlohmann::json::value_type& src)
	{
		if (!src.is_object()) {
			return { };
		}
		return { ToWSTRING(src.value("AssemblyPath", "")) , ToWSTRING(src.value("TypeName", "")), ToWSTRING(src.value("MethodName", "")), ToWSTRING(src.value("AssemblyName", "")) };
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
