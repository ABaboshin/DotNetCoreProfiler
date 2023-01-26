#include <algorithm>
#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>
#include "MethodRewriter.h"
#include "logging/logging.h"
#include "CorProfiler.h"
#include "util/helpers.h"
#include "const/const.h"

HRESULT MethodRewriter::EndTracing(rewriter::ILRewriterHelper& helper, rewriter::ILInstr** instr, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdTypeRef typeRef, const RejitInfo& interceptor, ULONG returnIndex, mdTypeRef exceptionTypeRef, ULONG exceptionIndex)
{
    HRESULT hr;

    auto isVoid = interceptor.Info.Signature.ReturnType.IsVoid;

    // define generic After method
    COR_SIGNATURE genericEndTracingSignature[1024];
    unsigned genericEndTracingSignatureOffset = 0;

    // see ECMA-355 II.23.2.15 MethodSpec
    genericEndTracingSignature[genericEndTracingSignatureOffset++] = IMAGE_CEE_CS_CALLCONV_GENERIC;
    // one generic parameter
    genericEndTracingSignature[genericEndTracingSignatureOffset++] = 1;
    // two method parameters
    genericEndTracingSignature[genericEndTracingSignatureOffset++] = 2;
    // return type
    genericEndTracingSignature[genericEndTracingSignatureOffset++] = ELEMENT_TYPE_VOID;
    // result type
    genericEndTracingSignature[genericEndTracingSignatureOffset++] = ELEMENT_TYPE_MVAR;
    genericEndTracingSignature[genericEndTracingSignatureOffset++] = 0;

    // exception
    unsigned exceptionTypeRefBuffer;
    auto exceptionTypeRefSize = CorSigCompressToken(exceptionTypeRef, &exceptionTypeRefBuffer);
    genericEndTracingSignature[genericEndTracingSignatureOffset++] = ELEMENT_TYPE_CLASS;
    memcpy(&genericEndTracingSignature[genericEndTracingSignatureOffset], &exceptionTypeRefBuffer, exceptionTypeRefSize);
    genericEndTracingSignatureOffset += exceptionTypeRefSize;

    mdMemberRef genericEndTracingRef;
    hr = metadataEmit->DefineMemberRef(
        typeRef,
        profiler->configuration.TracingEndMethod->MethodName.data(),
        genericEndTracingSignature,
        genericEndTracingSignatureOffset,
        &genericEndTracingRef);

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::ERR, "Failed CreateAfterMethod {0}"_W, interceptor.Info.Name);
        return hr;
    }

    // get result type id

    mdToken returnValueTypeSpec = mdTokenNil;
    auto returnSignature = interceptor.Info.Signature.ReturnType.Raw;
    if (!isVoid) {
        hr = metadataEmit->GetTokenFromTypeSpec(&returnSignature[0], returnSignature.size(), &returnValueTypeSpec);
    }

    // load this or null
    if (returnValueTypeSpec == mdTokenNil) {
        auto loadNull = helper.LoadNull();
        if (instr != nullptr) {
            *instr = loadNull;
        }
    }
    else
    {
        auto loadLocal = helper.LoadLocal(returnIndex);
        if (instr != nullptr) {
            *instr = loadLocal;
        }
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
            logging::log(logging::LogLevel::ERR, "Failed CreateAfterMethod GetObjectTypeRef"_W);
            return hr;
        }
        unsigned objectTypeRefBuffer = 0;
        auto objectTypeRefSize = CorSigCompressToken(objectTypeRef, &objectTypeRefBuffer);
        memcpy(&signature[offset], &objectTypeRefBuffer, objectTypeRefSize);
        offset += objectTypeRefSize;
    }

    mdMethodSpec endTracingSpec = mdMethodSpecNil;
    hr = metadataEmit->DefineMethodSpec(genericEndTracingRef, signature,
        offset, &endTracingSpec);

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::ERR, "Failed CreateAfterMethod DefineMethodSpec"_W);
        return hr;
    }

    helper.CallMember(endTracingSpec, false);
    return S_OK;
}