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

HRESULT MethodRewriter::CreateAfterMethod(rewriter::ILRewriterHelper &helper, rewriter::ILInstr **instr, util::ComPtr<IMetaDataEmit2> &metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdTypeRef interceptorTypeRef, const RejitInfo &interceptor, ULONG returnIndex, mdTypeRef exceptionTypeRef, ULONG exceptionIndex)
{
    HRESULT hr;

    auto isVoid = interceptor.info.Signature.ReturnType.IsVoid;

    // define generic After method
    COR_SIGNATURE genericAfterSignature[1024];
    unsigned genericAfterSignatureOffset = 0;

    // see ECMA-355 II.23.2.15 MethodSpec
    genericAfterSignature[genericAfterSignatureOffset++] = IMAGE_CEE_CS_CALLCONV_GENERIC;
    // one generic parameter
    genericAfterSignature[genericAfterSignatureOffset++] = 1;
    // two method parameters
    genericAfterSignature[genericAfterSignatureOffset++] = 2;
    // return type
    genericAfterSignature[genericAfterSignatureOffset++] = ELEMENT_TYPE_VOID;
    // result type
    genericAfterSignature[genericAfterSignatureOffset++] = ELEMENT_TYPE_MVAR;
    genericAfterSignature[genericAfterSignatureOffset++] = 0;

    // exception
    unsigned exceptionTypeRefBuffer;
    auto exceptionTypeRefSize = CorSigCompressToken(exceptionTypeRef, &exceptionTypeRefBuffer);
    genericAfterSignature[genericAfterSignatureOffset++] = ELEMENT_TYPE_CLASS;
    memcpy(&genericAfterSignature[genericAfterSignatureOffset], &exceptionTypeRefBuffer, exceptionTypeRefSize);
    genericAfterSignatureOffset += exceptionTypeRefSize;

    mdMemberRef genericAfterRef;
    hr = metadataEmit->DefineMemberRef(
        interceptorTypeRef,
        _const::AfterMethod.data(),
        genericAfterSignature,
        genericAfterSignatureOffset,
        &genericAfterRef);

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateAfterMethod {0}"_W, interceptor.interceptor.Interceptor.TypeName);
        return hr;
    }

    // get result type id

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

    // see ECMA-355 II.23.2.15 MethodSpec
    signature[offset++] = IMAGE_CEE_CS_CALLCONV_GENERICINST;
    // one generic parameter
    signature[offset++] = 1;

    // if not void => use signature
    if (!isVoid) {
        memcpy(&signature[offset], &returnSignature[0], returnSignature.size());
        offset += returnSignature.size();
    }
    else {
        // else use System.Object
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

    mdMethodSpec afterMethodSpec = mdMethodSpecNil;
    hr = metadataEmit->DefineMethodSpec(genericAfterRef, signature,
        offset, &afterMethodSpec);

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateAfterMethod DefineMethodSpec"_W);
        return hr;
    }

    helper.CallMember(afterMethodSpec, false);
    return S_OK;
}
