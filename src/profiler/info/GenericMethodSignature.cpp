#include "GenericMethodSignature.h"

#include "info/parser.h"

namespace info
{
	GenericMethodSignature::GenericMethodSignature(std::vector<BYTE> raw) : Raw(raw)
	{
		auto iter = this->Raw.begin();
		ULONG skip = 0;
		ParseNumber(iter, skip);

		ULONG number = 0;
		ParseNumber(iter, number);

		for (size_t i = 0; i < number; i++)
		{
			auto begin = iter;
			if (!ParseType(iter))
			{
				break;
			}

			Generics.push_back(TypeInfo(std::vector<BYTE>(begin, iter)));
		}
	}
}