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
	std::pair<InterceptorMethodInfo, bool> LoadInterceptorMethodFromJson(const nlohmann::json::value_type& src, bool required);

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
		std::vector<TraceMethodInfo> traces{};

		nlohmann::json j;
		stream >> j;

		auto loader = LoadTypeInfoFromJson(j["Loader"]);
		auto defaultInitializer = LoadInterceptorMethodFromJson(j["DefaultInitializer"], true);
		auto exceptionLogger = LoadInterceptorMethodFromJson(j["ExceptionLogger"], false);

		auto tracingBeginMethod = LoadInterceptorMethodFromJson(j["TracingBeginMethod"], false);
		auto tracingEndMethod = LoadInterceptorMethodFromJson(j["TracingEndMethod"], false);
		auto tracingAddParameterMethod = LoadInterceptorMethodFromJson(j["TracingAddParameterMethod"], false);

		auto debuggerBeginMethod = LoadInterceptorMethodFromJson(j["DebuggerBeginMethod"], false);
		auto debuggerEndMethod = LoadInterceptorMethodFromJson(j["DebuggerEndMethod"], false);
		auto debuggerAddParameterMethod = LoadInterceptorMethodFromJson(j["DebuggerAddParameterMethod"], false);

		for (auto& el : j["Strict"]) {
			auto i = LoadInterceptionFromJson(el);
			if (std::get<1>(i)) {
				interceptions.push_back(std::get<0>(i));
			}
		}

		for (auto& el : j["Traces"]) {
			auto tm = LoadTargetFromJson(el["TargetMethod"]);
			if (std::get<1>(tm)) {
				std::vector<wstring> parameters{};
				for (const auto& p : el["Parameters"]) {
					parameters.push_back(util::ToWSTRING(p));
				}
				
				traces.push_back({ std::get<0>(tm) , parameters, util::ToWSTRING(el.value("Name", ""))});
			}
		}

		for (auto& el : j["SkipAssemblies"]) {
			skipAssemblies.insert(ToWSTRING(el));
		}

		std::unordered_map<TargetMethod, InstrumentationConfiguration> instrumentations{};

		for (size_t i = 0; i < traces.size(); i++)
		{
			auto it = instrumentations.find(traces[i].TargetMethod);
			if (it == instrumentations.end())
			{
				std::pair<TargetMethod, InstrumentationConfiguration> ti(traces[i].TargetMethod, {});
				instrumentations.insert(ti);

				it = instrumentations.find(traces[i].TargetMethod);
			}

			it->second.Parameters = traces[i].Parameters;
			it->second.TraceName = traces[i].Name;
			it->second.Trace = true;
		}

		for (size_t i = 0; i < interceptions.size(); i++)
		{
			auto it = instrumentations.find(interceptions[i].TargetMethod);
			if (it == instrumentations.end())
			{
				std::pair<TargetMethod, InstrumentationConfiguration> ti(interceptions[i].TargetMethod, {});
				instrumentations.insert(ti);

				it = instrumentations.find(interceptions[i].TargetMethod);
			}

			it->second.Interceptors.push_back(interceptions[i]);
		}

		return {
			skipAssemblies,
			loader.first	,
			defaultInitializer.first,
			exceptionLogger.second ? std::make_shared<InterceptorMethodInfo>(exceptionLogger.first) : nullptr,
			tracingBeginMethod.second ? std::make_shared<InterceptorMethodInfo>(tracingBeginMethod.first) : nullptr,
			tracingEndMethod.second ? std::make_shared<InterceptorMethodInfo>(tracingEndMethod.first) : nullptr,
			tracingAddParameterMethod.second ? std::make_shared<InterceptorMethodInfo>(tracingAddParameterMethod.first) : nullptr,
			debuggerBeginMethod.second ? std::make_shared<InterceptorMethodInfo>(debuggerBeginMethod.first) : nullptr,
			debuggerEndMethod.second ? std::make_shared<InterceptorMethodInfo>(debuggerEndMethod.first) : nullptr,
			debuggerAddParameterMethod.second ? std::make_shared<InterceptorMethodInfo>(debuggerAddParameterMethod.first) : nullptr,
			instrumentations
		};
	}

	std::pair<TypeInfo, bool> LoadTypeInfoFromJson(const nlohmann::json::value_type& src)
	{
		if (!src.is_object()) {
			return std::make_pair<TypeInfo, bool>({}, false);
		}

		auto assemblyName = ToWSTRING(src.value("AssemblyName", ""));
		auto typeName = ToWSTRING(src.value("TypeName", ""));
		auto assemblyPath = ToWSTRING(src.value("AssemblyPath", ""));

		return std::make_pair<TypeInfo, bool>({ assemblyName , typeName, assemblyPath }, true);
	}

	std::pair<StrictInterception, bool> LoadInterceptionFromJson(const nlohmann::json::value_type& src)
	{
		if (!src.is_object()) {
			return std::make_pair<StrictInterception, bool>({}, false);
		}

		auto interceptor = std::get<0>(LoadTypeInfoFromJson(src["Interceptor"]));
		auto target = std::get<0>(LoadTargetFromJson(src["TargetMethod"]));
		auto priority = src.value("Priority", 0);

		return std::make_pair<StrictInterception, bool>({ target, interceptor, priority}, true);
	}

	std::pair<InterceptorMethodInfo, bool> LoadInterceptorMethodFromJson(const nlohmann::json::value_type& src, bool required)
	{
		if (!src.is_object()) {
			if (required)
			{
				throw std::string("DefaultInitializer not found");
			}
			
			return std::make_pair<InterceptorMethodInfo, bool>({}, false);
		}

		return std::make_pair<InterceptorMethodInfo, bool>({ ToWSTRING(src.value("AssemblyPath", "")) , ToWSTRING(src.value("TypeName", "")), ToWSTRING(src.value("MethodName", "")), ToWSTRING(src.value("AssemblyName", "")) }, true);
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
