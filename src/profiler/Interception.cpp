#include <iostream>
#include "Interception.h"
#include "util.h"

std::vector<Interception> LoadFromFile(const WSTRING& path)
{
	std::cout << "Load from " << ToString(path) << std::endl;
	std::vector<Interception> interceptions;

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

	return interceptions;
}

std::vector<Interception> LoadFromStream(std::ifstream& stream)
{
	std::cout << "Load from stream" << std::endl;
	std::vector<Interception> interceptions;

	nlohmann::json j;
	stream >> j;

	for (auto& el : j) {
		auto i = LoadFromJson(el);
		std::cout << std::get<1>(i) << std::endl;
		if (std::get<1>(i)) {
			interceptions.push_back(std::get<0>(i));
		}
	}

	return interceptions;
}

std::pair<Interception, bool> LoadFromJson(const nlohmann::json::value_type& src)
{
	std::cout << "el " << src << std::endl;
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
	
	auto raw_signature = src.value("WrapperSignature", "");

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

	return std::make_pair<Interception, bool>({ CallerAssemblyName, TargetAssemblyName, TargetTypeName , TargetMethodName , WrapperAssemblyPath, WrapperAssemblyName , WrapperTypeName , WrapperMethodName, signature  }, true);
}

WSTRING ToString(const Interception& interception)
{
	return interception.TargetAssemblyName + " "_W +
		interception.TargetTypeName + " "_W +
		interception.TargetMethodName + " "_W +
		interception.WrapperAssemblyPath + " "_W +
		interception.WrapperAssemblyName + " "_W +
		interception.WrapperTypeName + " "_W +
		interception.WrapperMethodName + " "_W;
}