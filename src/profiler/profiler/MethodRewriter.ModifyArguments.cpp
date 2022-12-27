#include <algorithm>
#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>
#include "MethodRewriter.h"
#include "logging/logging.h"
#include "CorProfiler.h"
#include "util/helpers.h"
#include "const/const.h"

HRESULT MethodRewriter::ModifyArguments(rewriter::ILRewriterHelper& helper, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdTypeRef interceptorTypeRef, const RejitInfo& interceptor)
{
	if (interceptor.info.Signature.NumberOfArguments() == 0) return S_OK;

    // define generic Before method
    std::vector<BYTE> genericBeforeSignature = {
        // see ECMA-355 II.23.2.15 MethodSpec
        IMAGE_CEE_CS_CALLCONV_GENERIC,
        // num of generic arguments
        // num of all arguments
        1,
        2,
        // return type
        ELEMENT_TYPE_VOID,
        ELEMENT_TYPE_I4,
        ELEMENT_TYPE_BYREF,
        ELEMENT_TYPE_MVAR,
        0
    };

    mdMemberRef genericBeforeRef;
    auto hr = metadataEmit->DefineMemberRef(
        interceptorTypeRef,
        _const::ModifyArgument.data(),
        genericBeforeSignature.data(),
        genericBeforeSignature.size(),
        &genericBeforeRef);

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed ModifyArguments {0}"_W, interceptor.interceptor.Interceptor.TypeName);
        return hr;
    }

    for (auto i = 0; i < interceptor.info.Signature.NumberOfArguments(); i++)
    {
        COR_SIGNATURE signature[1024];
        unsigned offset = 0;

        signature[offset++] = IMAGE_CEE_CS_CALLCONV_GENERICINST;
        // number of generic parameters
        signature[offset++] = 1;

        if (interceptor.info.Signature.Arguments[i].Raw[0] == ELEMENT_TYPE_BYREF) {
            memcpy(&signature[offset], &interceptor.info.Signature.Arguments[i].Raw[1], interceptor.info.Signature.Arguments[i].Raw.size() - 1);
            offset += interceptor.info.Signature.Arguments[i].Raw.size();
        }
        else
        {
            memcpy(&signature[offset], &interceptor.info.Signature.Arguments[i].Raw[0], interceptor.info.Signature.Arguments[i].Raw.size());
            offset += interceptor.info.Signature.Arguments[i].Raw.size();
        }

        mdMethodSpec beforeMethodSpec = mdMethodSpecNil;
        hr = metadataEmit->DefineMethodSpec(genericBeforeRef, signature,
            offset, &beforeMethodSpec);

        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateBeforeMethod DefineMethodSpec"_W);
            return hr;
        }

        helper.LoadInt32(i);
        helper.LoadArgumentRef(i);

        helper.CallMember(beforeMethodSpec, false);
    }

	return S_OK;
}