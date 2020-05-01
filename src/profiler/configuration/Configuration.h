#pragma once

#include <vector>
#include "Interception.h"

namespace configuration
{
	class Configuration
	{
	public:
		std::vector<Interception> interceptions{};

		Configuration(std::vector<Interception> interceptions) :
			interceptions(interceptions) {}

		Configuration() {}
	};
}
