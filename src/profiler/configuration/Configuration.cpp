#include <iostream>
#include <fstream>
#include <utility>
#include "nlohmann/json.hpp"
#include "configuration/Configuration.h"

namespace configuration
{
	Configuration LoadFromStream(std::ifstream& stream);
	std::pair<StrictInterception, bool> LoadInterceptionFromJson(const nlohmann::json::value_type& src);
	std::pair<Interceptor, bool> LoadInterceptorFromJson(const nlohmann::json::value_type& src);
	std::pair<TargetMethod, bool> LoadTargetFromJson(const nlohmann::json::value_type& src);
	std::pair<AttributedInterceptor, bool> LoadAttributedInterceptorFromJson(const nlohmann::json::value_type& src);

	Configuration Configuration::LoadConfiguration(const wstring& path)
	{
		Configuration configuration;

		try
		{
			std::ifstream stream;
			stream.open(ToString(path));

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
		std::vector<AttributedInterceptor> attributedInterceptors{};

		nlohmann::json j;
		stream >> j;

		for (auto& el : j["strict"]) {
			auto i = LoadInterceptionFromJson(el);
			if (std::get<1>(i)) {
				interceptions.push_back(std::get<0>(i));
			}
		}

		for (auto& el : j["assemblies"]) {
			assemblies.push_back(ToWSTRING(el));
		}

		for (auto& el : j["attributed"]) {
			auto i = LoadAttributedInterceptorFromJson(el);
			if (std::get<1>(i)) {
				attributedInterceptors.push_back(std::get<0>(i));
			}
		}

		return { interceptions, assemblies, attributedInterceptors };
	}

	std::pair<AttributedInterceptor, bool> LoadAttributedInterceptorFromJson(const nlohmann::json::value_type& src)
	{
		if (!src.is_object()) {
			return std::make_pair<AttributedInterceptor, bool>({}, false);
		}

		auto attributeType = ToWSTRING(src.value("AttributeType", ""));
		auto interceptor = std::get<0>(LoadInterceptorFromJson(src["Interceptor"]));

		return std::make_pair<AttributedInterceptor, bool>({ interceptor, attributeType }, true);
	}

	std::pair<StrictInterception, bool> LoadInterceptionFromJson(const nlohmann::json::value_type& src)
	{
		if (!src.is_object()) {
			return std::make_pair<StrictInterception, bool>({}, false);
		}

		auto callerAssemblyName = ToWSTRING(src.value("CallerAssemblyName", ""));
		auto interceptor = std::get<0>(LoadInterceptorFromJson(src["Interceptor"]));
		auto target = std::get<0>(LoadTargetFromJson(src["Target"]));

		return std::make_pair<StrictInterception, bool>({ callerAssemblyName, target, interceptor }, true);
	}

	std::pair<Interceptor, bool> LoadInterceptorFromJson(const nlohmann::json::value_type& src)
	{
		if (!src.is_object()) {
			return std::make_pair<Interceptor, bool>({}, false);
		}

		auto assemblyName = ToWSTRING(src.value("AssemblyName", ""));
		auto typeName = ToWSTRING(src.value("TypeName", ""));

		return std::make_pair<Interceptor, bool>({ assemblyName , typeName }, true);
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