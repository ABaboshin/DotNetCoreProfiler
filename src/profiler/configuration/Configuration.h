#pragma once
#include <vector>
#include "Interception.h"

class Configuration
{
public:
	std::vector<Interception> interceptions{};

	Configuration(std::vector<Interception> interceptions) :
		interceptions(interceptions) {}

	Configuration() {}
};
