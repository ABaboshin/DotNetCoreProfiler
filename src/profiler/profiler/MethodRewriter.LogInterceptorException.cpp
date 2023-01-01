#include <algorithm>
#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>
#include "MethodRewriter.h"
#include "logging/logging.h"
#include "CorProfiler.h"
#include "util/helpers.h"
#include "const/const.h"

HRESULT MethodRewriter::LogInterceptorException(rewriter::ILRewriterHelper& helper, rewriter::ILInstr** instr, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdTypeRef exceptionTypeRef)
{
    // if no exception logger is defined => do nothing
    if (profiler->configuration.ExceptionLogger == nullptr) {
        *instr = helper.Nop();
        return S_OK;
    }
    // define interceptor.dll
    mdModuleRef baseDllRef;
    auto hr = GetAssemblyRef(metadataAssemblyEmit, baseDllRef, profiler->configuration.ExceptionLogger->AssemblyName);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed GetWrapperRef {0}"_W, profiler->configuration.ExceptionLogger->AssemblyName);
        return hr;
    }

    // define ExceptionLogger type
    mdTypeRef defaultInitializerTypeRef;
    hr = metadataEmit->DefineTypeRefByName(
        baseDllRef,
        profiler->configuration.ExceptionLogger->TypeName.data(),
        &defaultInitializerTypeRef);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed DefineTypeRefByName {0}"_W, profiler->configuration.ExceptionLogger->TypeName);
        return hr;
    }

    COR_SIGNATURE memberSignature[1024];
    unsigned offset = 0;
    memberSignature[offset++] = IMAGE_CEE_CS_CALLCONV_DEFAULT;
    memberSignature[offset++] = 1;
    memberSignature[offset++] = ELEMENT_TYPE_VOID;
    memberSignature[offset++] = ELEMENT_TYPE_CLASS;

    // exception
    unsigned exceptionTypeRefBuffer;
    auto exceptionTypeRefSize = CorSigCompressToken(exceptionTypeRef, &exceptionTypeRefBuffer);
    memcpy(&memberSignature[offset], &exceptionTypeRefBuffer, exceptionTypeRefSize);
    offset += exceptionTypeRefSize;

    mdMemberRef loggerRef;
    hr = metadataEmit->DefineMemberRef(
        defaultInitializerTypeRef, profiler->configuration.ExceptionLogger->MethodName.data(),
        memberSignature,
        offset,
        &loggerRef);

    *instr = helper.CallMember(loggerRef, false);

    return S_OK;
}