#include "parser.h"

namespace info {
    bool ParseTypeDefOrRefEncoded(std::vector<BYTE>::iterator& begin) {
        mdToken typeToken;
        ULONG result;
        result = CorSigUncompressToken(&*begin, &typeToken);
        if (result == -1) {
            return false;
        }

        std::advance(begin, result);

        return true;
    }

    bool ParseCustomMod(std::vector<BYTE>::iterator& begin) {
        if (*begin == ELEMENT_TYPE_CMOD_OPT || *begin == ELEMENT_TYPE_CMOD_REQD) {
            std::advance(begin, 1);
            return ParseTypeDefOrRefEncoded(begin);
        }

        return false;
    }

    bool ParseOptionalCustomMods(std::vector<BYTE>::iterator& begin) {
        while (true) {
            switch (*begin) {
            case ELEMENT_TYPE_CMOD_OPT:
            case ELEMENT_TYPE_CMOD_REQD:
                if (!ParseCustomMod(begin)) {
                    return false;
                }
                break;
            default:
                return true;
            }
        }

        return false;
    }

    bool ParseNumber(std::vector<BYTE>::iterator& begin, ULONG& number) {
        ULONG result = CorSigUncompressData(&*begin, &number);
        if (result == -1) {
            return false;
        }

        std::advance(begin, result);

        return true;
    }

    bool ParseParam(std::vector<BYTE>::iterator& begin) {
        if (!ParseOptionalCustomMods(begin)) {
            return false;
        }

        if (*begin == ELEMENT_TYPE_TYPEDBYREF) {
            std::advance(begin, 1);
            return true;
        }

        if (*begin == ELEMENT_TYPE_BYREF) {
            std::advance(begin, 1);
        }

        return ParseType(begin);
    }

    bool ParseMethod(std::vector<BYTE>::iterator& begin) {
        if (*begin == IMAGE_CEE_CS_CALLCONV_GENERIC) {
            std::advance(begin, 1);

            ULONG genericCount = 0;
            if (!ParseNumber(begin, genericCount)) {
                return false;
            }
        }

        ULONG paramCount = 0;
        if (!ParseNumber(begin, paramCount)) {
            return false;
        }

        if (!ParseRetType(begin)) {
            return false;
        }

        bool sentinelFound = false;
        for (ULONG i = 0; i < paramCount; i++) {
            if (*begin  == ELEMENT_TYPE_SENTINEL) {
                if (sentinelFound) {
                    return false;
                }

                sentinelFound = true;
                std::advance(begin, 1);
            }


            if (!ParseParam(begin)) {
                return false;
            }
        }

        return true;
    }

    bool ParseArrayShape(std::vector<BYTE>::iterator& begin) {
        // Format: Rank NumSizes Size* NumLoBounds LoBound*
        ULONG rank = 0, numsizes = 0, size = 0;
        if (!ParseNumber(begin, rank) || !ParseNumber(begin, numsizes)) {
            return false;
        }

        for (auto i = 0; i < numsizes; i++) {
            if (!ParseNumber(begin, size)) {
                return false;
            }
        }

        if (!ParseNumber(begin, numsizes)) {
            return false;
        }

        for (auto i = 0; i < numsizes; i++) {
            if (!ParseNumber(begin, size)) {
                return false;
            }
        }

        return true;
    }

    bool ParseType(std::vector<BYTE>::iterator& begin) {

        auto cor_element_type = *begin;
        ULONG number = 0;
        std::advance(begin, 1);

        switch (cor_element_type) {
        case ELEMENT_TYPE_VOID:
        case ELEMENT_TYPE_BOOLEAN:
        case ELEMENT_TYPE_CHAR:
        case ELEMENT_TYPE_I1:
        case ELEMENT_TYPE_U1:
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
        case ELEMENT_TYPE_R4:
        case ELEMENT_TYPE_R8:
        case ELEMENT_TYPE_STRING:
        case ELEMENT_TYPE_OBJECT:
            return true;

        case ELEMENT_TYPE_PTR:
            if (!ParseOptionalCustomMods(begin)) {
                return false;
            }

            if (*begin == ELEMENT_TYPE_VOID) {
                std::advance(begin, 1);
                return true;
            }
            else {
                return ParseType(begin);
            }

        case ELEMENT_TYPE_VALUETYPE:
        case ELEMENT_TYPE_CLASS:
            return ParseTypeDefOrRefEncoded(begin);

        case ELEMENT_TYPE_FNPTR:
            return ParseMethod(begin);

        case ELEMENT_TYPE_ARRAY:
            // Format: ARRAY Type ArrayShape
            if (!ParseType(begin)) {
                return false;
            }
            return ParseArrayShape(begin);

        case ELEMENT_TYPE_SZARRAY:
            // Format: SZARRAY CustomMod* Type
            if (!ParseOptionalCustomMods(begin)) {
                return false;
            }
            return ParseType(begin);

        case ELEMENT_TYPE_GENERICINST:
            if (*begin != ELEMENT_TYPE_VALUETYPE && *begin != ELEMENT_TYPE_CLASS) {
                return false;
            }

            std::advance(begin, 1);
            if (!ParseTypeDefOrRefEncoded(begin)) {
                return false;
            }

            if (!ParseNumber(begin, number)) {
                return false;
            }

            for (auto i = 0; i < number; i++) {
                if (!ParseType(begin)) {
                    return false;
                }
            }
            return true;

        case ELEMENT_TYPE_VAR:
        case ELEMENT_TYPE_MVAR:
            // Format: VAR Number
            // Format: MVAR Number
            return ParseNumber(begin, number);

        default:
            return false;
        }
    }

    bool ParseRetType(std::vector<BYTE>::iterator& begin)
    {
        if (!ParseOptionalCustomMods(begin))
        {
            return false;
        }

        if (*begin == ELEMENT_TYPE_TYPEDBYREF || *begin == ELEMENT_TYPE_VOID)
        {
            std::advance(begin, 1);
            return true;
        }

        if (*begin == ELEMENT_TYPE_BYREF)
        {
            std::advance(begin, 1);
        }

        return ParseType(begin);
    }

    bool IsVoid(const std::vector<BYTE>& type)
    {
        return type.size() == 1 && type[0] == ELEMENT_TYPE_VOID;
    }
}