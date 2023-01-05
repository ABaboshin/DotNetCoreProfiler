#include <algorithm>
#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>
#include "MethodRewriter.h"
#include "logging/logging.h"
#include "CorProfiler.h"
#include "util/helpers.h"
#include "const/const.h"
#include "info/parser.h"

struct greater
{
	template<class T>
	bool operator()(T const& a, T const& b) const { return a > b; }
};

HRESULT MethodRewriter::AddDebugger(rewriter::ILRewriterHelper& helper, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, const ComPtr<IMetaDataImport2>& metadataImport, const RejitInfo& interceptor, mdTypeRef exceptionTypeRef, std::vector<BYTE>& origlocalSignature)
{
    HRESULT hr;
	if (interceptor.Offsets.empty())
	{
		return S_OK;
	}

	auto offsets = interceptor.Offsets;

    mdTypeRef debuggerBeginMethodRef = mdTokenNil;
    mdTypeRef debuggerEndMethodRef = mdTokenNil;
    mdTypeRef debuggerAddParameterMethodRef = mdTokenNil;
    if (profiler->configuration.DebuggerBeginMethod != nullptr && profiler->configuration.DebuggerEndMethod != nullptr && profiler->configuration.DebuggerAddParameterMethod != nullptr)
    {
        hr = FindTypeRef(metadataEmit, metadataAssemblyEmit, profiler->configuration.DebuggerBeginMethod->AssemblyName, profiler->configuration.DebuggerBeginMethod->TypeName, debuggerBeginMethodRef);
        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed FindTypeRef {0} {1}"_W, profiler->configuration.DebuggerBeginMethod->AssemblyName, profiler->configuration.DebuggerBeginMethod->TypeName);
            return hr;
        }

        hr = FindTypeRef(metadataEmit, metadataAssemblyEmit, profiler->configuration.DebuggerEndMethod->AssemblyName, profiler->configuration.DebuggerEndMethod->TypeName, debuggerEndMethodRef);
        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed FindTypeRef {0} {1}"_W, profiler->configuration.DebuggerEndMethod->AssemblyName, profiler->configuration.DebuggerEndMethod->TypeName);
            return hr;
        }

        hr = FindTypeRef(metadataEmit, metadataAssemblyEmit, profiler->configuration.DebuggerAddParameterMethod->AssemblyName, profiler->configuration.DebuggerAddParameterMethod->TypeName, debuggerAddParameterMethodRef);
        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed FindTypeRef {0} {1}"_W, profiler->configuration.DebuggerAddParameterMethod->AssemblyName, profiler->configuration.DebuggerAddParameterMethod->TypeName);
            return hr;
        }
    }
    else
    {
        logging::log(logging::LogLevel::INFO, "Skip debugger injecting as no debugger defined"_W);

        // if no debugger methods are defined => skip
        return S_OK;
    }

	// insert debugger from the end to start
	// to preserve the offsets
	std::sort(offsets.begin(), offsets.end(), greater());

    auto iter = origlocalSignature.begin();
    BYTE sigType = 0;
    info::ParseByte(iter, sigType);

    if (sigType != IMAGE_CEE_CS_CALLCONV_LOCAL_SIG)
    {
        return S_FALSE;
    }

    ULONG localsCount = 0;
    if (!info::ParseNumber(iter, localsCount))
    {
        return S_FALSE;
    }

    std::vector<info::TypeInfo> locals{};

    for (auto i = 0; i < localsCount; i++)
    {
        auto begin = iter;
        if (!info::ParseParam(iter))
        {
            return S_FALSE;
        }

        info::TypeInfo ti(std::vector<BYTE>(begin, iter));
        locals.push_back(ti);
    }

	for (const auto el : offsets)
	{
		auto hr = AddDebugger(helper, metadataEmit, metadataAssemblyEmit, metadataImport, interceptor, exceptionTypeRef, locals, debuggerBeginMethodRef, debuggerEndMethodRef, debuggerAddParameterMethodRef, el);
		if (FAILED(hr))
		{
			return hr;
		}
	}

    rewriter::ILInstr* offsetInstr = helper._rewriter->GetInstrFromOffset(0);
    helper.SetILPosition(offsetInstr);

	return S_OK;
}

HRESULT MethodRewriter::AddDebugger(rewriter::ILRewriterHelper& helper, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, const ComPtr<IMetaDataImport2>& metadataImport, const RejitInfo& interceptor, mdTypeRef exceptionTypeRef, const std::vector<info::TypeInfo>& locals, mdTypeRef debuggerBeginMethodRef, mdTypeRef debuggerEndMethodRef, mdTypeRef debuggerAddParameterMethodRef, int offset)
{
    HRESULT hr;

    rewriter::ILInstr* offsetInstr = helper._rewriter->GetInstrFromOffset(offset);

    if (offsetInstr == nullptr)
    {
        return S_FALSE;
    }

    if (offsetInstr->m_opcode == rewriter::CEE_NOP || offsetInstr->m_opcode == rewriter::CEE_BR_S || offsetInstr->m_opcode == rewriter::CEE_BR)
    {
        offsetInstr = offsetInstr->m_pNext;
    }

    auto prevInstr = offsetInstr->m_pPrev;

    helper.SetILPosition(offsetInstr);

    //begin debug
    {
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
            debuggerBeginMethodRef,
            profiler->configuration.DebuggerBeginMethod->MethodName.data(),
            beginSignature.data(),
            beginSignature.size(),
            &beginRef);

        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateBeforeMethod {0}"_W, profiler->configuration.DebuggerBeginMethod->MethodName);
            return hr;
        }

        mdString nameToken;
        util::wstring name = "debug"_W;
        hr = metadataEmit->DefineUserString(name.c_str(), (ULONG)name.length(), &nameToken);
        helper.LoadStr(nameToken);
        helper.CallMember(beginRef, false);
    }

    // define generic Before method
    std::vector<BYTE> genericAddParameterSignature = {
        // see ECMA-355 II.23.2.15 MethodSpec
        IMAGE_CEE_CS_CALLCONV_GENERIC,
        // num of generic arguments
        // num of all arguments
        1,
        3,
        // return type
        ELEMENT_TYPE_VOID,
        ELEMENT_TYPE_STRING,
        ELEMENT_TYPE_STRING,
        ELEMENT_TYPE_MVAR,
        0
    };

    mdMemberRef genericAddParameterRef;
    hr = metadataEmit->DefineMemberRef(
        debuggerAddParameterMethodRef,
        profiler->configuration.DebuggerAddParameterMethod->MethodName.data(),
        genericAddParameterSignature.data(),
        genericAddParameterSignature.size(),
        &genericAddParameterRef);

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateBeforeMethod {0}"_W, profiler->configuration.DebuggerAddParameterMethod->MethodName);
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

    mdString parameterTypeNameToken;
    wstring parameterTypeName = "parameter"_W;
    hr = metadataEmit->DefineUserString(parameterTypeName.c_str(), (ULONG)parameterTypeName.length(), &parameterTypeNameToken);

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
        wstring parameterName = interceptor.Parameters.size() > i ? interceptor.Parameters[i + (interceptor.Info.Signature.IsInstanceMethod() ? 1 : 0)] : util::ToWSTRING(std::to_string(i));
        hr = metadataEmit->DefineUserString(parameterName.c_str(), (ULONG)parameterName.length(), &parameterNameToken);

        helper.LoadStr(parameterTypeNameToken);
        helper.LoadStr(parameterNameToken);
        // load argument
        helper.LoadArgument(i + (interceptor.Info.Signature.IsInstanceMethod() ? 1 : 0));

        helper.CallMember(addParameterSpec, false);
    }

    mdString varTypeNameToken;
    wstring varTypeName = "var"_W;
    hr = metadataEmit->DefineUserString(varTypeName.c_str(), (ULONG)varTypeName.length(), &varTypeNameToken);

    

    for (auto i = 0; i < interceptor.Variables.size(); i++)
    {
        COR_SIGNATURE signature[1024];
        unsigned offset = 0;

        signature[offset++] = IMAGE_CEE_CS_CALLCONV_GENERICINST;
        // number of generic parameters
        signature[offset++] = 1;
        memcpy(&signature[offset], &locals[i].Raw[0], locals[i].Raw.size());
        offset += locals[i].Raw.size();

        mdMethodSpec addParameterSpec = mdMethodSpecNil;
        hr = metadataEmit->DefineMethodSpec(genericAddParameterRef, signature,
            offset, &addParameterSpec);

        mdString parameterNameToken;
        wstring parameterName = interceptor.Variables.size() >= i ? interceptor.Variables[i] : util::ToWSTRING(std::to_string(i));
        hr = metadataEmit->DefineUserString(parameterName.c_str(), (ULONG)parameterName.length(), &parameterNameToken);

        helper.LoadStr(varTypeNameToken);
        helper.LoadStr(parameterNameToken);
        // load local
        helper.LoadLocal(i);

        helper.CallMember(addParameterSpec, false);
    }

    // end debug
    {
        // define before method
        std::vector<BYTE> endSignature = {
            IMAGE_CEE_CS_CALLCONV_DEFAULT,
            0,
            // return type
            ELEMENT_TYPE_VOID
        };

        mdMemberRef endRef;
        hr = metadataEmit->DefineMemberRef(
            debuggerBeginMethodRef,
            profiler->configuration.DebuggerEndMethod->MethodName.data(),
            endSignature.data(),
            endSignature.size(),
            &endRef);

        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateAfterMethod {0}"_W, profiler->configuration.DebuggerBeginMethod->MethodName);
            return hr;
        }

        helper.CallMember(endRef, false);
    }

    return S_OK;
}