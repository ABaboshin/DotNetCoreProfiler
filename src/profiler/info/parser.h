#pragma once
#include <vector>
#include "cor.h"

namespace info {
	bool ParseRetType(std::vector<BYTE>::iterator& begin);
	bool ParseType(std::vector<BYTE>::iterator& begin);
	bool ParseParam(std::vector<BYTE>::iterator& begin);
}