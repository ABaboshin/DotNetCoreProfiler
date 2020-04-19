#pragma once
#include <vector>
#include "Interception.h"
#include "Initializer.h"

class Configuration
{
public:
	std::vector<Interception> interceptions;
	Initializer initializer;

	Configuration(std::vector<Interception> interceptions, Initializer initializer) :
		interceptions(interceptions),
		initializer(initializer) {}

	Configuration() {}
};

Configuration LoadFromFile(const WSTRING& path);