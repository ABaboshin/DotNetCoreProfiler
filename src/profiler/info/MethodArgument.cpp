#include "MethodArgument.h"
#include "MethodArgumentTypeFlag.h"
#include "util/clr_const.h"
#include "util/helpers.h"

int MethodArgument::GetTypeFlags(unsigned& elementType) const {

    int flag = 0;
    PCCOR_SIGNATURE pbCur = &pbBase[offset];

    if (*pbCur == ELEMENT_TYPE_VOID) {
        elementType = ELEMENT_TYPE_VOID;
        flag |= TypeFlagVoid;
        return flag;
    }

    if (*pbCur == ELEMENT_TYPE_BYREF) {
        pbCur++;
        flag |= TypeFlagByRef;
    }

    elementType = *pbCur;

    switch (*pbCur) {
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
    case  ELEMENT_TYPE_VALUETYPE:
    case  ELEMENT_TYPE_MVAR:
    case  ELEMENT_TYPE_VAR:
        flag |= TypeFlagBoxedType;
        break;
    case  ELEMENT_TYPE_GENERICINST:
        pbCur++;
        if (*pbCur == ELEMENT_TYPE_VALUETYPE) {
            flag |= TypeFlagBoxedType;
        }
        break;
    default:
        break;
    }
    return flag;
}

mdToken MethodArgument::GetTypeTok(const ComPtr<IMetaDataEmit2> pEmit,
    mdAssemblyRef corLibRef) const {

    mdToken token = mdTokenNil;
    PCCOR_SIGNATURE pbCur = &pbBase[offset];
    const PCCOR_SIGNATURE pTemp = pbCur;

    if (*pbCur == ELEMENT_TYPE_BYREF) {
        pbCur++;
    }

    switch (*pbCur) {
    case  ELEMENT_TYPE_BOOLEAN:
        pEmit->DefineTypeRefByName(corLibRef, SystemBoolean.data(), &token);
        break;
    case  ELEMENT_TYPE_CHAR:
        pEmit->DefineTypeRefByName(corLibRef, SystemChar.data(), &token);
        break;
    case  ELEMENT_TYPE_I1:
        pEmit->DefineTypeRefByName(corLibRef, SystemByte.data(), &token);
        break;
    case  ELEMENT_TYPE_U1:
        pEmit->DefineTypeRefByName(corLibRef, SystemSByte.data(), &token);
        break;
    case  ELEMENT_TYPE_U2:
        pEmit->DefineTypeRefByName(corLibRef, SystemUInt16.data(), &token);
        break;
    case  ELEMENT_TYPE_I2:
        pEmit->DefineTypeRefByName(corLibRef, SystemInt16.data(), &token);
        break;
    case  ELEMENT_TYPE_I4:
        pEmit->DefineTypeRefByName(corLibRef, SystemInt32.data(), &token);
        break;
    case  ELEMENT_TYPE_U4:
        pEmit->DefineTypeRefByName(corLibRef, SystemUInt32.data(), &token);
        break;
    case  ELEMENT_TYPE_I8:
        pEmit->DefineTypeRefByName(corLibRef, SystemInt64.data(), &token);
        break;
    case  ELEMENT_TYPE_U8:
        pEmit->DefineTypeRefByName(corLibRef, SystemUInt64.data(), &token);
        break;
    case  ELEMENT_TYPE_R4:
        pEmit->DefineTypeRefByName(corLibRef, SystemSingle.data(), &token);
        break;
    case  ELEMENT_TYPE_R8:
        pEmit->DefineTypeRefByName(corLibRef, SystemDouble.data(), &token);
        break;
    case  ELEMENT_TYPE_I:
        pEmit->DefineTypeRefByName(corLibRef, SystemIntPtr.data(), &token);
        break;
    case  ELEMENT_TYPE_U:
        pEmit->DefineTypeRefByName(corLibRef, SystemUIntPtr.data(), &token);
        break;
    case  ELEMENT_TYPE_STRING:
        pEmit->DefineTypeRefByName(corLibRef, SystemString.data(), &token);
        break;
    case  ELEMENT_TYPE_OBJECT:
        pEmit->DefineTypeRefByName(corLibRef, SystemObject.data(), &token);
        break;
    case  ELEMENT_TYPE_CLASS:
        pbCur++;
        token = CorSigUncompressToken(pbCur);
        break;
    case  ELEMENT_TYPE_VALUETYPE:
        pbCur++;
        token = CorSigUncompressToken(pbCur);
        break;
    case  ELEMENT_TYPE_GENERICINST:
    case  ELEMENT_TYPE_SZARRAY:
    case  ELEMENT_TYPE_MVAR:
    case  ELEMENT_TYPE_VAR:
        pEmit->GetTokenFromTypeSpec(pbCur, length - static_cast<ULONG>(pbCur - pTemp), &token);
        break;
    default:
        break;
    }
    return token;
}

wstring MethodArgument::GetTypeTokName(ComPtr<IMetaDataImport2>& pImport) const
{
    PCCOR_SIGNATURE pbCur = &pbBase[offset];
    return GetSigTypeTokName(pbCur, pImport);
}