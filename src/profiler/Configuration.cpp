#include "Configuration.h"
#include <iostream>

Configuration LoadFromStream(std::ifstream& stream);
std::pair<Interception, bool> LoadInterceptionFromJson(const nlohmann::json::value_type& src);
std::pair<Initializer, bool> LoadInitializerFromJson(const nlohmann::json::value_type& src);
std::vector<BYTE> ReadSignature(std::string raw_signature);

Configuration LoadFromFile(const WSTRING& path)
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

	auto CallerAssemblyName = ToWSTRING(src.value("CallerAssemblyName", ""));
	auto TargetAssemblyName = ToWSTRING(src.value("TargetAssemblyName", ""));
	auto TargetTypeName = ToWSTRING(src.value("TargetTypeName", ""));
	auto TargetMethodName = ToWSTRING(src.value("TargetMethodName", ""));
	auto WrapperAssemblyPath = ToWSTRING(src.value("WrapperAssemblyPath", ""));
	auto WrapperAssemblyName = ToWSTRING(src.value("WrapperAssemblyName", ""));
	auto WrapperTypeName = ToWSTRING(src.value("WrapperTypeName", ""));
	auto WrapperMethodName = ToWSTRING(src.value("WrapperMethodName", ""));
	auto TargetMethodParametersCount = src.value("TargetMethodParametersCount", 0);

	auto signature = ReadSignature(src.value("WrapperSignature", ""));

	return std::make_pair<Interception, bool>({ CallerAssemblyName, TargetAssemblyName, TargetTypeName , TargetMethodName , WrapperAssemblyPath, WrapperAssemblyName , WrapperTypeName , WrapperMethodName, signature, TargetMethodParametersCount }, true);
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