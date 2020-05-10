#include <iostream>
#include <string>
#include "cor.h"
#include "corhlpr.h"

#include "FunctionInfo.h"
#include "MethodSignature.h"
#include "TypeInfo.h"
#include "parser.h"
#include "const/const.h"
#include "util/helpers.h"

namespace info
{
    MethodSignature::MethodSignature(std::vector<BYTE> raw) : raw(raw)
    {
        auto begin= this->raw.begin() + 2 + (IsGeneric() ? 1 : 0);
        auto iter = begin;
        if (ParseRetType(iter))
        {
            auto distance = std::distance(begin, iter);
            ret = std::vector<BYTE>(begin, iter);

            argumentsOffset = std::distance(this->raw.begin(), iter);
        }

        /*for (size_t i = 0; i < NumberOfArguments(); i++)
        {
            begin = iter;
            ParseParam(iter);
            auto distance = std::distance(begin, iter);
            auto param = std::vector<BYTE>(begin, iter);

            std::cout << "Parse " << i << " from " << NumberOfArguments() << " bytes " << param.size() << std::endl;

            for (size_t i = 0; i < param.size(); i++)
            {
                std::cout << (int)param[i] << std::endl;
            }
        }*/
    }

    void MethodSignature::ParseArguments()
    {
        auto iter = raw.begin();
        std::advance(iter, argumentsOffset);
        
        for (size_t i = 0; i < NumberOfArguments(); i++)
        {
            auto begin = iter;
            if (!ParseParam(iter))
            {
                break;
            }

            arguments.push_back(MethodArgument(std::vector<BYTE>(begin, iter)));
        }
    }
}