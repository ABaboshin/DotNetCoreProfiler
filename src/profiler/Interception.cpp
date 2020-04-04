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

	auto AssemblyName = ToWSTRING(src.value("AssemblyName", ""));
	auto TypeName = ToWSTRING(src.value("TypeName", ""));
	auto MethodName = ToWSTRING(src.value("MethodName", ""));
	auto WrapperAssemblyPath = ToWSTRING(src.value("WrapperAssemblyPath", ""));
	auto WrapperAssemblyName = ToWSTRING(src.value("WrapperAssemblyName", ""));
	auto WrapperTypeName = ToWSTRING(src.value("WrapperTypeName", ""));
	auto WrapperMethodName = ToWSTRING(src.value("WrapperMethodName", ""));
	auto isCounter = src.value("isCounter", false);

	return std::make_pair<Interception, bool>({ AssemblyName, TypeName , MethodName , isCounter , WrapperAssemblyPath, WrapperAssemblyName , WrapperTypeName , WrapperMethodName  }, true);
}

WSTRING ToString(const Interception& interception)
{
	return interception.AssemblyName + " "_W +
		interception.TypeName + " "_W +
		interception.MethodName + " "_W +
		(interception.isCounter ? "true"_W: "false"_W) + " "_W +
		interception.WrapperAssemblyPath + " "_W +
		interception.WrapperAssemblyName + " "_W +
		interception.WrapperTypeName + " "_W +
		interception.WrapperMethodName + " "_W;
}