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
    std::vector<BYTE> MethodSignature::GetRet() {
        auto beginRetSignature = raw.begin() + 2 + (IsGeneric() ? 1 : 0);
        auto iter = beginRetSignature;
        if (ParseRetType(iter))
        {
            auto distance = std::distance(beginRetSignature, iter);
            auto result = std::vector<BYTE>(beginRetSignature, iter);
            return result;
        }
        else
        {
            return std::vector<BYTE>();
        }
    }
}