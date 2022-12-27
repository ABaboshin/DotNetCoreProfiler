#include <algorithm>
#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>
#include "MethodRewriter.h"
#include "logging/logging.h"
#include "CorProfiler.h"
#include "util/helpers.h"
#include "const/const.h"

HRESULT MethodRewriter::CreateBeforeMethod(rewriter::ILRewriterHelper& helper, rewriter::ILInstr** instr, util::ComPtr< IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdTypeRef interceptorTypeRef, const RejitInfo& interceptor)
{
    HRESULT hr;

    auto interceptorNumberOfArguments = interceptor.info.Signature.NumberOfArguments() + 1;

    // define generic Before method
    std::vector<BYTE> genericBeforeSignature = {
        // see ECMA-355 II.23.2.15 MethodSpec
        IMAGE_CEE_CS_CALLCONV_GENERIC,
        // num of generic arguments
        // num of all arguments
        (BYTE)interceptorNumberOfArguments,
        (BYTE)interceptorNumberOfArguments,
        // return type
        ELEMENT_TYPE_VOID};

    for (auto i = 0; i < interceptorNumberOfArguments; i++)
    {
        if (i > 0) {
            genericBeforeSignature.push_back(ELEMENT_TYPE_BYREF);
        }
        
        genericBeforeSignature.push_back(ELEMENT_TYPE_MVAR);
        genericBeforeSignature.push_back((BYTE)i);
    }

    mdMemberRef genericBeforeRef;
    hr = metadataEmit->DefineMemberRef(
        interceptorTypeRef,
        _const::BeforeMethod.data(),
        genericBeforeSignature.data(),
        genericBeforeSignature.size(),
        &genericBeforeRef);

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateBeforeMethod {0}"_W, interceptor.interceptor.Interceptor.TypeName);
        return hr;
    }

    // get target type id

    auto isValueType = false;
    mdToken targetTypeRef = mdTokenNil;
    if (interceptor.info.Signature.IsInstanceMethod())
    {
        hr = GetTargetTypeRef(interceptor.info.Type, metadataEmit, metadataAssemblyEmit, &targetTypeRef, &isValueType);
        if (FAILED(hr)) {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateBeforeMethod {0}"_W, interceptor.interceptor.Interceptor.TypeName);
            return hr;
        }
    }

    // load this or null
    if (targetTypeRef == mdTokenNil || !interceptor.info.Signature.IsInstanceMethod()) {
        *instr = helper.LoadNull();
    }
    else
    {
        *instr = helper.LoadArgument(0);
        if (interceptor.info.Type.IsValueType) {
            if (interceptor.info.Type.TypeSpec != mdTypeSpecNil) {
                helper.LoadObj(interceptor.info.Type.TypeSpec);
            }
            else if (interceptor.info.Type.IsGenericClassRef) {
                helper.LoadObj(interceptor.info.Type.Id);
            }
            else {
                return S_FALSE;
            }
        }
    }

    // load arguments
    for (auto i = 0; i < interceptor.info.Signature.NumberOfArguments(); i++)
    {
        helper.LoadArgumentRef(i + (interceptor.info.Signature.IsInstanceMethod() ? 1 : 0));
    }

    // non generic Before method
    if (targetTypeRef == mdTokenNil) {
        hr = GetObjectTypeRef(metadataEmit, metadataAssemblyEmit, &targetTypeRef);
        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateBeforeMethod GetObjectTypeRef"_W);
            return hr;
        }
    }

    unsigned targetTypeBuffer = 0;
    ULONG targetTypeSize = CorSigCompressToken(targetTypeRef, &targetTypeBuffer);

    COR_SIGNATURE signature[1024];
    unsigned offset = 0;

    signature[offset++] = IMAGE_CEE_CS_CALLCONV_GENERICINST;
    // number of generic parameters
    signature[offset++] = interceptorNumberOfArguments;

    if (isValueType) {
        signature[offset++] = ELEMENT_TYPE_VALUETYPE;
    }
    else {
        signature[offset++] = ELEMENT_TYPE_CLASS;
    }

    memcpy(&signature[offset], &targetTypeBuffer, targetTypeSize);
    offset += targetTypeSize;

    for (auto i = 0; i < interceptor.info.Signature.NumberOfArguments(); i++)
    {
        if (interceptor.info.Signature.Arguments[i].Raw[0] == ELEMENT_TYPE_BYREF) {
            memcpy(&signature[offset], &interceptor.info.Signature.Arguments[i].Raw[1], interceptor.info.Signature.Arguments[i].Raw.size() - 1);
            offset += interceptor.info.Signature.Arguments[i].Raw.size();
        }
        else
        {
            memcpy(&signature[offset], &interceptor.info.Signature.Arguments[i].Raw[0], interceptor.info.Signature.Arguments[i].Raw.size());
            offset += interceptor.info.Signature.Arguments[i].Raw.size();
        }
    }

    mdMethodSpec beforeMethodSpec = mdMethodSpecNil;
    hr = metadataEmit->DefineMethodSpec(genericBeforeRef, signature,
        offset, &beforeMethodSpec);

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateBeforeMethod DefineMethodSpec"_W);
        return hr;
    }

    helper.CallMember(beforeMethodSpec, false);
    return S_OK;
}
