#include <iostream>
#include <string>
#include "corhlpr.h"

#include "FunctionInfo.h"
#include "MethodSignature.h"
#include "TypeInfo.h"
#include "const/const.h"
#include "util/helpers.h"

bool ParseType(PCCOR_SIGNATURE& pbCur, PCCOR_SIGNATURE pbEnd);
bool ParseByte(PCCOR_SIGNATURE& pbCur, PCCOR_SIGNATURE pbEnd, unsigned char* pbOut);
bool ParseTypeDefOrRefEncoded(PCCOR_SIGNATURE& pbCur, PCCOR_SIGNATURE pbEnd,
    unsigned char* pIndexTypeOut, unsigned* pIndexOut);
bool ParseNumber(PCCOR_SIGNATURE& pbCur, PCCOR_SIGNATURE pbEnd, unsigned* pOut);

/*  we don't support
        PTR CustomMod* VOID
        PTR CustomMod* Type
        FNPTR MethodDefSig
        FNPTR MethodRefSig
        ARRAY Type ArrayShape
        CustomMod*
     */
bool ParseType(PCCOR_SIGNATURE& pbCur, PCCOR_SIGNATURE pbEnd)
{
    /*
    Type ::= ( BOOLEAN | CHAR | I1 | U1 | U2 | U2 | I4 | U4 | I8 | U8 | R4 | R8 | I | U |
    | VALUETYPE TypeDefOrRefEncoded
    | CLASS TypeDefOrRefEncoded
    | STRING
    | OBJECT
    | PTR CustomMod* VOID
    | PTR CustomMod* Type
    | FNPTR MethodDefSig
    | FNPTR MethodRefSig
    | ARRAY Type ArrayShape
    | SZARRAY CustomMod* Type
    | GENERICINST (CLASS | VALUETYPE) TypeDefOrRefEncoded GenArgCount Type *
    | VAR Number
    | MVAR Number

    */

    unsigned char elem_type;
    unsigned index;
    unsigned number;
    unsigned char indexType;

    if (!ParseByte(pbCur, pbEnd, &elem_type))
        return false;

    switch (elem_type)
    {
    case  ELEMENT_TYPE_BOOLEAN:
    case  ELEMENT_TYPE_CHAR:
    case  ELEMENT_TYPE_I1:
    case  ELEMENT_TYPE_U1:
    case  ELEMENT_TYPE_U2:
    case  ELEMENT_TYPE_I2:
    case  ELEMENT_TYPE_I4:
    case  ELEMENT_TYPE_U4:
    case  ELEMENT_TYPE_I8:
    case  ELEMENT_TYPE_U8:
    case  ELEMENT_TYPE_R4:
    case  ELEMENT_TYPE_R8:
    case  ELEMENT_TYPE_I:
    case  ELEMENT_TYPE_U:
    case  ELEMENT_TYPE_STRING:
    case  ELEMENT_TYPE_OBJECT:
        // simple types
        break;

    case  ELEMENT_TYPE_PTR:

        return false;

    case  ELEMENT_TYPE_CLASS:
        // CLASS TypeDefOrRefEncoded

        if (!ParseTypeDefOrRefEncoded(pbCur, pbEnd, &indexType, &index))
            return false;
        break;

    case  ELEMENT_TYPE_VALUETYPE:
        //VALUETYPE TypeDefOrRefEncoded

        if (!ParseTypeDefOrRefEncoded(pbCur, pbEnd, &indexType, &index))
            return false;

        break;

    case  ELEMENT_TYPE_FNPTR:
        // FNPTR MethodDefSig
        // FNPTR MethodRefSig

        return false;

    case  ELEMENT_TYPE_ARRAY:
        // ARRAY Type ArrayShape
        return false;

    case  ELEMENT_TYPE_SZARRAY:
        // SZARRAY Type

        if (*pbCur == ELEMENT_TYPE_CMOD_OPT || *pbCur == ELEMENT_TYPE_CMOD_REQD) {
            return false;
        }

        if (!ParseType(pbCur, pbEnd))
            return false;

        break;

    case  ELEMENT_TYPE_GENERICINST:
        // GENERICINST (CLASS | VALUETYPE) TypeDefOrRefEncoded GenArgCount Type *

        if (!ParseByte(pbCur, pbEnd, &elem_type))
            return false;

        if (elem_type != ELEMENT_TYPE_CLASS && elem_type != ELEMENT_TYPE_VALUETYPE)
            return false;

        if (!ParseTypeDefOrRefEncoded(pbCur, pbEnd, &indexType, &index))
            return false;

        if (!ParseNumber(pbCur, pbEnd, &number))
            return false;

        for (unsigned i = 0; i < number; i++)
        {
            if (!ParseType(pbCur, pbEnd))
                return false;
        }

        break;

    case  ELEMENT_TYPE_VAR:
        // VAR Number
        if (!ParseNumber(pbCur, pbEnd, &number))
            return false;

        break;

    case  ELEMENT_TYPE_MVAR:
        // MVAR Number
        if (!ParseNumber(pbCur, pbEnd, &number))
            return false;

        break;
    }

    return true;
}

bool ParseByte(PCCOR_SIGNATURE& pbCur, PCCOR_SIGNATURE pbEnd, unsigned char* pbOut)
{
    if (pbCur < pbEnd)
    {
        *pbOut = *pbCur;
        pbCur++;
        return true;
    }

    return false;
}

bool ParseNumber(PCCOR_SIGNATURE& pbCur, PCCOR_SIGNATURE pbEnd, unsigned* pOut)
{
    // parse the variable length number format (0-4 bytes)

    unsigned char b1 = 0, b2 = 0, b3 = 0, b4 = 0;

    // at least one byte in the encoding, read that

    if (!ParseByte(pbCur, pbEnd, &b1))
        return false;

    if (b1 == 0xff)
    {
        // special encoding of 'NULL'
        // not sure what this means as a number, don't expect to see it except for string lengths
        // which we don't encounter anyway so calling it an error
        return false;
    }

    // early out on 1 byte encoding
    if ((b1 & 0x80) == 0)
    {
        *pOut = (int)b1;
        return true;
    }

    // now at least 2 bytes in the encoding, read 2nd byte
    if (!ParseByte(pbCur, pbEnd, &b2))
        return false;

    // early out on 2 byte encoding
    if ((b1 & 0x40) == 0)
    {
        *pOut = (((b1 & 0x3f) << 8) | b2);
        return true;
    }

    // must be a 4 byte encoding

    if ((b1 & 0x20) != 0)
    {
        // 4 byte encoding has this bit clear -- error if not
        return false;
    }

    if (!ParseByte(pbCur, pbEnd, &b3))
        return false;

    if (!ParseByte(pbCur, pbEnd, &b4))
        return false;

    *pOut = ((b1 & 0x1f) << 24) | (b2 << 16) | (b3 << 8) | b4;
    return true;
}

// RetType ::= CustomMod* ( VOID | TYPEDBYREF | [BYREF] Type ) 
    // CustomMod* TYPEDBYREF we don't support
bool ParseRetType(PCCOR_SIGNATURE& pbCur, PCCOR_SIGNATURE pbEnd)
{
    if (*pbCur == ELEMENT_TYPE_CMOD_OPT || *pbCur == ELEMENT_TYPE_CMOD_REQD) {
        return false;
    }

    if (pbCur >= pbEnd)
        return false;

    if (*pbCur == ELEMENT_TYPE_TYPEDBYREF)
    {
        return false;
    }

    if (*pbCur == ELEMENT_TYPE_VOID)
    {
        pbCur++;
        return true;
    }

    if (*pbCur == ELEMENT_TYPE_BYREF)
    {
        pbCur++;
    }

    if (!ParseType(pbCur, pbEnd))
        return false;

    return true;
}

// Param ::= CustomMod* ( TYPEDBYREF | [BYREF] Type ) 
// CustomMod* TYPEDBYREF we don't support
bool ParseParam(PCCOR_SIGNATURE& pbCur, PCCOR_SIGNATURE pbEnd)
{
    if (*pbCur == ELEMENT_TYPE_CMOD_OPT || *pbCur == ELEMENT_TYPE_CMOD_REQD) {
        return false;
    }

    if (pbCur >= pbEnd)
        return false;

    if (*pbCur == ELEMENT_TYPE_TYPEDBYREF)
    {
        return false;
    }

    if (*pbCur == ELEMENT_TYPE_BYREF)
    {
        pbCur++;
    }

    if (!ParseType(pbCur, pbEnd))
        return false;

    return true;
}

bool ParseTypeDefOrRefEncoded(PCCOR_SIGNATURE& pbCur, PCCOR_SIGNATURE pbEnd,
    unsigned char* pIndexTypeOut, unsigned* pIndexOut)
{
    // parse an encoded typedef or typeref
    unsigned encoded = 0;

    if (!ParseNumber(pbCur, pbEnd, &encoded))
        return false;

    *pIndexTypeOut = (unsigned char)(encoded & 0x3);
    *pIndexOut = (encoded >> 2);
    return true;
}

HRESULT MethodSignature::TryParse() {
    HRESULT hr;
    PCCOR_SIGNATURE pbCur = pbBase;
    PCCOR_SIGNATURE pbEnd = pbBase + len;
    unsigned char elem_type;
    IfFailRet(ParseByte(pbCur, pbEnd, &elem_type));
    if (elem_type & IMAGE_CEE_CS_CALLCONV_GENERIC) {
        unsigned gen_param_count;
        IfFailRet(ParseNumber(pbCur, pbEnd, &gen_param_count));
        numberOfTypeArguments = gen_param_count;
    }

    unsigned param_count;
    IfFailRet(ParseNumber(pbCur, pbEnd, &param_count));
    numberOfArguments = param_count;

    const PCCOR_SIGNATURE pbRet = pbCur;

    IfFailRet(ParseRetType(pbCur, pbEnd));

    ret.pbBase = pbBase;
    ret.length = (ULONG)(pbCur - pbRet);
    ret.offset = (ULONG)(pbCur - pbBase - ret.length);

    auto fEncounteredSentinal = false;
    for (unsigned i = 0; i < param_count; i++) {
        if (pbCur >= pbEnd)
            return E_FAIL;

        if (*pbCur == ELEMENT_TYPE_SENTINEL) {
            if (fEncounteredSentinal)
                return E_FAIL;

            fEncounteredSentinal = true;
            pbCur++;
        }

        const PCCOR_SIGNATURE pbParam = pbCur;

        IfFailRet(ParseParam(pbCur, pbEnd));

        MethodArgument argument{};
        argument.pbBase = pbBase;
        argument.length = (ULONG)(pbCur - pbParam);
        argument.offset = (ULONG)(pbCur - pbBase - argument.length);

        params.push_back(argument);
    }

    return S_OK;
}

