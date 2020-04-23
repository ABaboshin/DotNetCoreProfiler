#include "Configuration.h"
#include <iostream>
#include <fstream>
#include <utility>
#include "nlohmann/json.hpp"

Configuration LoadFromStream(std::ifstream& stream);
std::pair<Interception, bool> LoadInterceptionFromJson(const nlohmann::json::value_type& src);
std::pair<Initializer, bool> LoadInitializerFromJson(const nlohmann::json::value_type& src);
std::pair<TargetMethod, bool> LoadTargetFromJson(const nlohmann::json::value_type& src);
std::pair<Interceptor, bool> LoadInterceptorFromJson(const nlohmann::json::value_type& src);
std::vector<BYTE> ReadSignature(std::string raw_signature);

Configuration LoadFromFile(const wstring& path)
{
	std::cout << "Load from " << ToString(path) << std::endl;
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
	std::vector<Interception> interceptions;
	Initializer initializer;

	nlohmann::json j;
	stream >> j;

	for (auto& el : j["interceptions"]) {
		auto i = LoadInterceptionFromJson(el);
		if (std::get<1>(i)) {
			interceptions.push_back(std::get<0>(i));
		}
	}

	{
		auto i = LoadInitializerFromJson(j["initializer"]);
		if (std::get<1>(i))
		{
			initializer = std::get<0>(i);
		}
	}

	return {interceptions, initializer };
}

std::pair<Interception, bool> LoadInterceptionFromJson(const nlohmann::json::value_type& src)
{
	if (!src.is_object()) {
		return std::make_pair<Interception, bool>({}, false);
	}

	auto callerAssemblyName = ToWSTRING(src.value("CallerAssemblyName", ""));
	auto interceptor = std::get<0>(LoadInterceptorFromJson(src["Interceptor"]));
	auto target = std::get<0>(LoadTargetFromJson(src["Target"]));

	return std::make_pair<Interception, bool>({ callerAssemblyName, target, interceptor }, true);
}

std::pair<Interceptor, bool> LoadInterceptorFromJson(const nlohmann::json::value_type& src)
{
	if (!src.is_object()) {
		return std::make_pair<Interceptor, bool>({}, false);
	}

	auto wrapperAssemblyPath = ToWSTRING(src.value("AssemblyPath", ""));
	auto wrapperAssemblyName = ToWSTRING(src.value("AssemblyName", ""));
	auto wrapperTypeName = ToWSTRING(src.value("TypeName", ""));
	auto wrapperMethodName = ToWSTRING(src.value("MethodName", ""));

	auto signature = ReadSignature(src.value("Signature", ""));

	return std::make_pair<Interceptor, bool>({ wrapperAssemblyPath, wrapperAssemblyName , wrapperTypeName , wrapperMethodName, signature }, true);
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

std::pair<Initializer, bool> LoadInitializerFromJson(const nlohmann::json::value_type& src)
{
	if (!src.is_object()) {
		return std::make_pair<Initializer, bool>({}, false);
	}

	auto assemblyPath = ToWSTRING(src.value("AssemblyPath", ""));
	auto assemblyName = ToWSTRING(src.value("AssemblyName", ""));
	auto typeName = ToWSTRING(src.value("TypeName", ""));

	return std::make_pair<Initializer, bool>({ assemblyPath, assemblyName , typeName }, true);
}

std::vector<BYTE> ReadSignature(std::string raw_signature)
{
	std::vector<BYTE> signature;

	// load as a hex string
	bool flip = false;
	char prev = 0;
	for (auto& c : raw_signature) {
		BYTE b = 0;
		if ('0' <= c && c <= '9') {
			b = c - '0';
		}
		else if ('a' <= c && c <= 'f') {
			b = c - 'a' + 10;
		}
		else if ('A' <= c && c <= 'F') {
			b = c - 'A' + 10;
		}
		else {
			// skip any non-hex character
			continue;
		}
		if (flip) {
			signature.push_back((prev << 4) + b);
		}
		flip = !flip;
		prev = b;
	}

	return signature;
}