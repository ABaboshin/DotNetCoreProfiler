#pragma once

namespace info
{
    enum MethodArgumentTypeFlag
    {
        TypeFlagByRef = 0x01,
        TypeFlagVoid = 0x02,
        TypeFlagBoxedType = 0x04
    };
}