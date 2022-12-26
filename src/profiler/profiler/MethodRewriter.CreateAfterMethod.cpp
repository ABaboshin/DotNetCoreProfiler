#include <algorithm>
#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>
#include "MethodRewriter.h"
#include "logging/logging.h"
#include "CorProfiler.h"
#include "util/helpers.h"
#include "const/const.h"
#include <vector>

//TODO
HRESULT MethodRewriter::CreateAfterMethod(rewriter::ILRewriterHelper &helper, rewriter::ILInstr **instr, util::ComPtr<IMetaDataEmit2> &metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdTypeRef interceptorTypeRef, const RejitInfo &interceptor, ULONG returnIndex, mdTypeRef exceptionTypeRef, ULONG exceptionIndex)
{
    //DebugBreak();

    HRESULT hr;

    auto isVoid = interceptor.info.Signature.ReturnType.IsVoid;

    std::cout << 1 << std::endl;

    // define generic After method
    COR_SIGNATURE genericAfterSignature[1024];
    unsigned genericAfterSignatureOffset = 0;
        /*
    std::vector<BYTE> genericAfterSignature */
    //    = {
    //IMAGE_CEE_CS_CALLCONV_GENERIC,
    //// one generic parameter
    //    1,
    //    // tow method parameters
    //    2,
    //    // return type
    //    ELEMENT_TYPE_VOID,
    //    // result type
    //    ELEMENT_TYPE_MVAR,
    //    0
    //};

    genericAfterSignature[genericAfterSignatureOffset++] = IMAGE_CEE_CS_CALLCONV_GENERIC;
    genericAfterSignature[genericAfterSignatureOffset++] = 1;
    genericAfterSignature[genericAfterSignatureOffset++] = 2;
    genericAfterSignature[genericAfterSignatureOffset++] = ELEMENT_TYPE_VOID;
    genericAfterSignature[genericAfterSignatureOffset++] = ELEMENT_TYPE_MVAR;
    genericAfterSignature[genericAfterSignatureOffset++] = 0;

    //// result type
    //genericAfterSignature.push_back(ELEMENT_TYPE_MVAR);
    //genericAfterSignature.push_back((BYTE)0);

    std::cout << 2 << std::endl;

    // exception
    unsigned exceptionTypeRefBuffer;
    auto exceptionTypeRefSize = CorSigCompressToken(exceptionTypeRef, &exceptionTypeRefBuffer);
    genericAfterSignature[genericAfterSignatureOffset++] = ELEMENT_TYPE_CLASS;
    memcpy(&genericAfterSignature[genericAfterSignatureOffset], &exceptionTypeRefBuffer, exceptionTypeRefSize);
    genericAfterSignatureOffset += exceptionTypeRefSize;
    /*std::vector<BYTE> exceptionTypeRefBuffer(10);

    auto exceptionTypeRefSize = CorSigCompressToken(exceptionTypeRef, &exceptionTypeRefBuffer[0]);

    std::cout << 3 << std::endl;*/

    /*genericAfterSignature.push_back(ELEMENT_TYPE_CLASS);

    std::cout << "exceptionTypeRefSize " << exceptionTypeRefSize << " exceptionTypeRefBuffer " << exceptionTypeRefBuffer.size() << std::endl;

    std::copy_n(exceptionTypeRefBuffer.begin(), exceptionTypeRefSize, std::back_inserter(genericAfterSignature));*/

    std::cout << 4 << std::endl;
    mdMemberRef genericAfterRef;
    hr = metadataEmit->DefineMemberRef(
        interceptorTypeRef,
        _const::AfterMethod.data(),
        genericAfterSignature,
        genericAfterSignatureOffset,
        &genericAfterRef);

    std::cout << 5 << std::endl;

    std::cout << "generic sig" << std::endl;
    for (auto i = 0; i < genericAfterSignatureOffset; i++)
    {
        std::cout << std::hex << (int)genericAfterSignature[i] << std::endl;
    }

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateAfterMethod {0}"_W, interceptor.interceptor.Interceptor.TypeName);
        return hr;
    }

    // get result type id

    //auto isValueType = interceptor.info.Signature.ReturnType.IsValueType;
    mdToken returnValueTypeSpec = mdTokenNil;
    auto returnSignature = interceptor.info.Signature.ReturnType.Raw;
    if (!isVoid) {
        hr = metadataEmit->GetTokenFromTypeSpec(&returnSignature[0], returnSignature.size(), &returnValueTypeSpec);
    }

    // load this or null
    if (returnValueTypeSpec == mdTokenNil) {
        *instr = helper.LoadNull();
    }
    else
    {
        *instr = helper.LoadLocal(returnIndex);
    }

    // load exception
    helper.LoadLocal(exceptionIndex);

    COR_SIGNATURE signature[1024];
    unsigned offset = 0;

    signature[offset++] = IMAGE_CEE_CS_CALLCONV_GENERICINST;
    //signature[offset++] = interceptorNumberOfArguments;
    signature[offset++] = 1;
    //signature[offset++] = ELEMENT_TYPE_VOID;

    /*if (isValueType) {
        signature[offset++] = ELEMENT_TYPE_VALUETYPE;
    }
    else {
        signature[offset++] = ELEMENT_TYPE_CLASS;
    }*/

    if (!isVoid) {
        memcpy(&signature[offset], &returnSignature[0], returnSignature.size());
        offset += returnSignature.size();
    }
    else {
        signature[offset++] = ELEMENT_TYPE_CLASS;
        mdTypeRef objectTypeRef;
        hr = GetObjectTypeRef(metadataEmit, metadataAssemblyEmit, &objectTypeRef);
        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateAfterMethod GetObjectTypeRef"_W);
            return hr;
        }
        unsigned objectTypeRefBuffer = 0;
        auto objectTypeRefSize = CorSigCompressToken(objectTypeRef, &objectTypeRefBuffer);
        memcpy(&signature[offset], &objectTypeRefBuffer, objectTypeRefSize);
        offset += objectTypeRefSize;
    }

    //signature[offset++] = ELEMENT_TYPE_CLASS;
    //memcpy(&signature[offset], &exceptionTypeRefBuffer, exceptionTypeRefSize);
    //offset += exceptionTypeRefSize;
    

    /*for (auto i = 0; i < interceptor.info.Signature.NumberOfArguments(); i++)
    {
        memcpy(&signature[offset], &interceptor.info.Signature.Arguments[i].Raw[0], interceptor.info.Signature.Arguments[i].Raw.size());
        offset += interceptor.info.Signature.Arguments[i].Raw.size();
    }*/

    std::cout << "non generic sig" << std::endl;
    for (auto i = 0; i < offset; i++)
    {
        std::cout << std::hex << (int)signature[i] << std::endl;
    }

    std::cout << "offset " << offset << /*" length " << signatureLength <<*/ std::endl;

    mdMethodSpec afterMethodSpec = mdMethodSpecNil;
    hr = metadataEmit->DefineMethodSpec(genericAfterRef, signature,
        offset, &afterMethodSpec);

    std::cout << "DefineMethodSpec " << hr << " " << afterMethodSpec << std::endl;

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateAfterMethod DefineMethodSpec"_W);
        return hr;
    }

    helper.CallMember(afterMethodSpec, false);
    //*instr = helper.Nop();
    return S_OK;
}
