#include <algorithm>
#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>
#include "MethodRewriter.h"
#include "logging/logging.h"
#include "CorProfiler.h"
#include "util/helpers.h"
#include "const/const.h"

HRESULT MethodRewriter::BeginTracing(rewriter::ILRewriterHelper& helper, rewriter::ILInstr** instr, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdTypeRef beginTypeRef, mdTypeRef addParameterTypeRef, const RejitInfo& interceptor)
{
	HRESULT hr;

    // define before method
    std::vector<BYTE> beginSignature = {
        IMAGE_CEE_CS_CALLCONV_DEFAULT,
        1,
        // return type
        ELEMENT_TYPE_VOID,
        ELEMENT_TYPE_STRING
    };

    mdMemberRef beginRef;
    hr = metadataEmit->DefineMemberRef(
        beginTypeRef,
        profiler->configuration.TracingBeginMethod->MethodName.data(),
        beginSignature.data(),
        beginSignature.size(),
        &beginRef);

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateBeforeMethod {0}"_W, profiler->configuration.TracingBeginMethod->MethodName);
        return hr;
    }

    mdString nameToken;
    hr = metadataEmit->DefineUserString(interceptor.TraceMethodInfo->Name.c_str(), (ULONG)interceptor.TraceMethodInfo->Name.length(), &nameToken);
    *instr = helper.LoadStr(nameToken);
    helper.CallMember(beginRef, false);

    // define generic Before method
    std::vector<BYTE> genericAddParameterSignature = {
        // see ECMA-355 II.23.2.15 MethodSpec
        IMAGE_CEE_CS_CALLCONV_GENERIC,
        // num of generic arguments
        // num of all arguments
        1,
        2,
        // return type
        ELEMENT_TYPE_VOID,
        ELEMENT_TYPE_STRING,
        ELEMENT_TYPE_MVAR,
        0
    };

    mdMemberRef genericAddParameterRef;
    hr = metadataEmit->DefineMemberRef(
        addParameterTypeRef,
        profiler->configuration.TracingAddParameterMethod->MethodName.data(),
        genericAddParameterSignature.data(),
        genericAddParameterSignature.size(),
        &genericAddParameterRef);

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateBeforeMethod {0}"_W, profiler->configuration.TracingAddParameterMethod->MethodName);
        return hr;
    }

    auto isValueType = false;
    mdToken targetTypeRef = mdTokenNil;
    if (interceptor.Info.Signature.IsInstanceMethod())
    {
        hr = GetTargetTypeRef(interceptor.Info.Type, metadataEmit, metadataAssemblyEmit, &targetTypeRef, &isValueType);
        if (FAILED(hr)) {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateBeforeMethod {0}"_W, interceptor.Info.Name);
            return hr;
        }
    }

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

    // this
    {
        COR_SIGNATURE signature[1024];
        unsigned offset = 0;

        signature[offset++] = IMAGE_CEE_CS_CALLCONV_GENERICINST;
        // number of generic parameters
        signature[offset++] = 1;
        if (isValueType) {
            signature[offset++] = ELEMENT_TYPE_VALUETYPE;
        }
        else {
            signature[offset++] = ELEMENT_TYPE_CLASS;
        }

        memcpy(&signature[offset], &targetTypeBuffer, targetTypeSize);
        offset += targetTypeSize;

        mdMethodSpec addParameterSpec = mdMethodSpecNil;
        hr = metadataEmit->DefineMethodSpec(genericAddParameterRef, signature,
            offset, &addParameterSpec);

        mdString parameterNameToken;
        wstring parameterName = "this"_W;
        hr = metadataEmit->DefineUserString(parameterName.c_str(), (ULONG)parameterName.length(), &parameterNameToken);

        helper.LoadStr(parameterNameToken);
        // load this or null
        if (targetTypeRef == mdTokenNil || !interceptor.Info.Signature.IsInstanceMethod()) {
            auto loadNull = helper.LoadNull();
        }
        else
        {
            auto loadArg = helper.LoadArgument(0);
            if (interceptor.Info.Type.IsValueType) {
                if (interceptor.Info.Type.TypeSpec != mdTypeSpecNil) {
                    helper.LoadObj(interceptor.Info.Type.TypeSpec);
                }
                else if (interceptor.Info.Type.IsGenericClassRef) {
                    helper.LoadObj(interceptor.Info.Type.Id);
                }
                else {
                    return S_FALSE;
                }
            }
        }

        helper.CallMember(addParameterSpec, false);
    }

    for (auto i = 0; i < interceptor.Info.Signature.NumberOfArguments(); i++)
    {
        COR_SIGNATURE signature[1024];
        unsigned offset = 0;

        signature[offset++] = IMAGE_CEE_CS_CALLCONV_GENERICINST;
        // number of generic parameters
        signature[offset++] = 1;
        memcpy(&signature[offset], &interceptor.Info.Signature.Arguments[i].Raw[0], interceptor.Info.Signature.Arguments[i].Raw.size());
        offset += interceptor.Info.Signature.Arguments[i].Raw.size();

        mdMethodSpec addParameterSpec = mdMethodSpecNil;
        hr = metadataEmit->DefineMethodSpec(genericAddParameterRef, signature,
            offset, &addParameterSpec);

        mdString parameterNameToken;
        wstring parameterName = interceptor.TraceMethodInfo->Parameters.size() >= i ? interceptor.TraceMethodInfo->Parameters[i] : util::ToWSTRING(std::to_string(i));
        hr = metadataEmit->DefineUserString(parameterName.c_str(), (ULONG)parameterName.length(), &parameterNameToken);

        helper.LoadStr(parameterNameToken);
        // load argument
        helper.LoadArgument(i + (interceptor.Info.Signature.IsInstanceMethod() ? 1 : 0));

        helper.CallMember(addParameterSpec, false);
    }

    return S_OK;
}