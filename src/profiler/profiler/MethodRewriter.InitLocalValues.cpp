#include <algorithm>
#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>
#include "MethodRewriter.h"
#include "logging/logging.h"
#include "CorProfiler.h"
#include "util/helpers.h"
#include "const/const.h"

HRESULT MethodRewriter::InitLocalValues(rewriter::ILRewriterHelper& helper, rewriter::ILRewriter* rewriter, ModuleID moduleId, const RejitInfo& interceptor, ULONG exceptionIndex, ULONG returnIndex, mdModuleRef baseDllRef) {
    HRESULT hr;

    ComPtr<IUnknown> metadataInterfaces;
    hr = profiler->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf());
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::PROFILERERROR, "Failed InitLocalValues {0}"_W, interceptor.interceptor.Interceptor.TypeName);
        return hr;
    }

    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);
    auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);
    auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);

    // ex = null
    helper.LoadNull();
    helper.StLocal(exceptionIndex);

    auto isVoid = interceptor.info.Signature.ReturnType.IsVoid;
    //isVoid = true;

    if (!isVoid)
    {
        // define default initializer type
        mdTypeRef defaultInitializerTypeRef;
        hr = metadataEmit->DefineTypeRefByName(
            baseDllRef,
            profiler->configuration.DefaultInitializer.TypeName.data(),
            &defaultInitializerTypeRef);
        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::PROFILERERROR, "Failed DefineTypeRefByName {0}"_W, profiler->configuration.DefaultInitializer.TypeName);
            return hr;
        }

        std::vector<BYTE> memberSignature = {
        IMAGE_CEE_CS_CALLCONV_GENERIC,
        1,
        0,
        ELEMENT_TYPE_MVAR,
        0 };

        // define generic GetDefault method
        mdMemberRef getDefaultRef;
        hr = metadataEmit->DefineMemberRef(
            defaultInitializerTypeRef,
            _const::GetDefault.data(),
            memberSignature.data(),
            memberSignature.size(),
            &getDefaultRef);

        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::PROFILERERROR, "Failed DefineMemberRef {0}"_W, profiler->configuration.DefaultInitializer.TypeName);
            return hr;
        }

        auto methodArgumentSignatureSize = interceptor.info.Signature.ReturnType.Raw.size();
        auto signatureLength = 2 + methodArgumentSignatureSize;

        COR_SIGNATURE signature[500];
        unsigned offset = 0;
        signature[offset++] = IMAGE_CEE_CS_CALLCONV_GENERICINST;
        signature[offset++] = 0x01;

std::cout << "return type size " << interceptor.info.Signature.ReturnType.Raw.size() << std::endl;
for (auto i = 0; i < interceptor.info.Signature.ReturnType.Raw.size(); i++)
{
    std::cout << std::hex << (int)interceptor.info.Signature.ReturnType.Raw[i] << std::endl;
}

        memcpy(&signature[offset], &interceptor.info.Signature.ReturnType.Raw[0], interceptor.info.Signature.ReturnType.Raw.size());
        offset += methodArgumentSignatureSize;

        std::cout << "sig length " << signatureLength << std::endl;
for (auto i = 0; i < signatureLength; i++)
{
    std::cout << std::hex << (int)signature[i] << std::endl;
}

        mdMethodSpec getDefaultSpecRef = mdMethodSpecNil;
        hr = metadataEmit->DefineMethodSpec(getDefaultRef, signature, signatureLength,
                                                          &getDefaultSpecRef);

        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::PROFILERERROR, "Failed DefineMethodSpec {0}"_W, profiler->configuration.DefaultInitializer.TypeName);
            return hr;
        }

        helper.CallMember(getDefaultSpecRef, false);
        //helper.Pop();
        helper.StLocal(returnIndex);
    }

    return S_OK;
}
