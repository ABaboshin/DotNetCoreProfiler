//#include "MethodArgument.h"
//#include <corhlpr.h>
//
//namespace info
//{
//    MethodArgument::MethodArgument(const std::vector<BYTE>& raw) : raw(raw)
//    {
//        isRefType = raw.size() > 0 && raw[0] == ELEMENT_TYPE_BYREF;
//
//        isVoid = raw.size() > 0 && raw[0] == ELEMENT_TYPE_VOID;
//
//        auto shift = isRefType ? 1 : 0;
//
//        switch (raw[isRefType]) {
//        case ELEMENT_TYPE_VOID:
//        case ELEMENT_TYPE_BOOLEAN:
//        case ELEMENT_TYPE_CHAR:
//        case ELEMENT_TYPE_I1:
//        case ELEMENT_TYPE_U1:
//        case ELEMENT_TYPE_I2:
//        case ELEMENT_TYPE_U2:
//        case ELEMENT_TYPE_I4:
//        case ELEMENT_TYPE_U4:
//        case ELEMENT_TYPE_I8:
//        case ELEMENT_TYPE_U8:
//        case ELEMENT_TYPE_R4:
//        case ELEMENT_TYPE_R8:
//        case ELEMENT_TYPE_I:
//        case ELEMENT_TYPE_U:
//        //case ELEMENT_TYPE_VALUETYPE:
//        case ELEMENT_TYPE_MVAR:
//        case ELEMENT_TYPE_VAR:
//        //case ELEMENT_TYPE_STRING:
//            typeDef = raw[isRefType];
//            isBoxed = true;
//            break;
//        default:
//            typeDef = ELEMENT_TYPE_OBJECT;
//            isBoxed = false;
//            break;
//        }
//    }
//}