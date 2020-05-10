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
    MethodSignature::MethodSignature(const std::vector<BYTE>& raw) : raw(raw)
    {
        auto callingConvention = raw.empty() ? 0 : raw[0];

        isGeneric = raw.size() > 2 && (callingConvention & IMAGE_CEE_CS_CALLCONV_GENERIC) != 0;
        bool isInstanceMethod = (callingConvention & IMAGE_CEE_CS_CALLCONV_HASTHIS) != 0;

        auto iter = this->raw.begin();

        // number of arguments
        if (isGeneric)
        {
            std::advance(iter, 2);
            ParseNumber(iter, numberOfArguments);
        }

        if (raw.size() > 1)
        {
            std::advance(iter, 1);
            ParseNumber(iter, numberOfArguments);
        }

        // return Type
        auto beginRetSignature = this->raw.begin() + 2 + (isGeneric ? 1 : 0);
        iter = beginRetSignature;
        if (ParseRetType(iter))
        {
            auto distance = std::distance(beginRetSignature, iter);
            returnType = std::vector<BYTE>(beginRetSignature, iter);
        }

        //// arguments
        //bool sentinelFound = false;
        //for (size_t i = 0; i < numberOfArguments; i++)
        //{
        //    if (*iter == ELEMENT_TYPE_SENTINEL)
        //    {
        //        if (sentinelFound)
        //        {
        //            break;
        //        }
        //    
        //        sentinelFound = true;
        //        std::advance(iter, 1);
        //    }

        //    auto begin = iter;
        //    if (ParseParam(iter))
        //    {
        //        auto distance = std::distance(beginRetSignature, iter);
        //        arguments.push_back(MethodArgument(std::vector<BYTE>(begin, iter)));
        //    }
        //}
    }
}