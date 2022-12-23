#include <algorithm>
#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>
#include "MethodRewriter.h"
#include "logging/logging.h"
#include "CorProfiler.h"
#include "rewriter/ILRewriterHelper.h"
#include "util/helpers.h"
#include "const/const.h"

/*

The idea behind is to rewrite the original method in the following way

Exception methodException = null;
try {
    try {
        Interceptor.Before(method params);
    } catch()
    {
    // ignore interceptor's exceptions
    }

    .... original method body ...
} catch (Exception e) {
    methodException = e;
} finally {
    try {
        Interceptor.After(result if not void otherwise null, methodException);
    } catch()
    {
    // ignore interceptor's exceptions
    }
}

*/

HRESULT MethodRewriter::Rewriter(ModuleID moduleId, mdMethodDef methodId, ICorProfilerFunctionControl* pFunctionControl)
{
    HRESULT hr;
    mdToken functionToken;
    ClassID classId;

    const auto interceptor = std::find_if(profiler->rejitInfo.begin(), profiler->rejitInfo.end(), [moduleId, methodId](const RejitInfo& ri) {
        return ri.methodId == methodId && ri.moduleId == moduleId;
    });

    // no rejit info => exit
    if (interceptor == profiler->rejitInfo.end()) return S_OK;

    logging::log(logging::LogLevel::INFO, "GetReJITParameters {0}.{1}"_W, interceptor->info.Type.Name, interceptor->info.Name);

    auto rewriter = profiler->CreateILRewriter(pFunctionControl, moduleId, methodId);
    hr = rewriter->Import();
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::_ERROR, "GetReJITParameters rewriter->Import failed"_W);
        return S_FALSE;
    }

    rewriter::ILRewriterHelper helper(rewriter);
    helper.SetILPosition(rewriter->GetILList()->m_pNext);

    if (profiler->loadedModules.find(interceptor->interceptor.Interceptor.AssemblyName) == profiler->loadedModules.end())
    {
        logging::log(logging::LogLevel::VERBOSE, "Skip rejit because interceptor assembly is not loaded yet"_W);
        return S_FALSE;
    }
    else
    {
        logging::log(logging::LogLevel::VERBOSE, "Continue rejit as interceptor assembly is loaded {0}"_W, profiler->loadedModules[interceptor->interceptor.Interceptor.AssemblyName]);
    }

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(profiler->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));
    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);
    auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);
    auto assemblyImport = metadataInterfaces.As<IMetaDataAssemblyImport>(IID_IMetaDataEmit);
    auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);

    logging::log(logging::LogLevel::VERBOSE, "{0}"_W, profiler->ilDumper.DumpILCodes("rejit before ", rewriter, interceptor->info, metadataImport));

    std::cout << util::ToString(profiler->ilDumper.DumpILCodes("rejit before ", rewriter, interceptor->info, metadataImport)) << std::endl;

    // define mscorlib.dll
    mdModuleRef mscorlibRef;
    GetMsCorLibRef(hr, metadataAssemblyEmit, mscorlibRef);
    IfFailRet(hr);

    // Define System.Exception
    mdTypeRef exceptionTypeRef;
    metadataEmit->DefineTypeRefByName(mscorlibRef, _const::SystemException.data(), &exceptionTypeRef);

    //// modify local signature
    //// to add an exception and the return value
    //mdToken localVarSig = rewriter->GetTkLocalVarSig();
    //PCCOR_SIGNATURE originalSignature = nullptr;
    //ULONG originalSignatureSize = 0;

    //if (localVarSig != mdTokenNil) {
    //    IfFailRet(metadataImport->GetSigFromToken(localVarSig, &originalSignature, &originalSignatureSize));
    //}
    //
    //// exception type buffer and size
    //unsigned exceptionTypeRefBuffer;
    //auto exceptionTypeRefSize = CorSigCompressToken(exceptionTypeRef, &exceptionTypeRefBuffer);

    //// return type signature
    //ULONG returnSignatureTypeSize = 0;
    //mdTypeSpec returnValueTypeSpec = mdTypeSpecNil;

    //auto newSignatureSize = originalSignatureSize + (1 + exceptionTypeRefSize);
    //ULONG newSignatureOffset = 0;
    //ULONG newLocalsCount = 1;
    //if (!interceptor->info.Signature.ReturnType.IsVoid)
    //{
    //    returnSignatureTypeSize = interceptor->info.Signature.ReturnType.Raw.size();

    //    mdTypeSpec returnValueTypeSpec = mdTypeSpecNil;
    //    hr = metadataEmit->GetTokenFromTypeSpec(&interceptor->info.Signature.ReturnType.Raw[0], interceptor->info.Signature.ReturnType.Raw.size(), &returnValueTypeSpec);
    //    newSignatureSize += (1 + returnSignatureTypeSize);
    //    newLocalsCount++;
    //}

    //ULONG oldLocalsBuffer;
    //ULONG oldLocalsLen = 0;
    //unsigned newLocalsBuffer;
    //ULONG newLocalsLen;

    //if (originalSignatureSize == 0)
    //{
    //    newSignatureSize += 2;
    //    newLocalsLen = CorSigCompressData(newLocalsCount, &newLocalsBuffer);
    //}
    //else
    //{
    //    oldLocalsLen = CorSigUncompressData(originalSignature + 1, &oldLocalsBuffer);
    //    newLocalsCount += oldLocalsBuffer;
    //    newLocalsLen = CorSigCompressData(newLocalsCount, &newLocalsBuffer);
    //    newSignatureSize += newLocalsLen - oldLocalsLen;
    //}

    //// New signature declaration
    //COR_SIGNATURE newSignatureBuffer[500];
    //newSignatureBuffer[newSignatureOffset++] = IMAGE_CEE_CS_CALLCONV_LOCAL_SIG;

    //// Set the locals count
    //memcpy(&newSignatureBuffer[newSignatureOffset], &newLocalsBuffer, newLocalsLen);
    //newSignatureOffset += newLocalsLen;

    //if (originalSignatureSize > 0)
    //{
    //    const auto copyLength = originalSignatureSize - 1 - oldLocalsLen;
    //    memcpy(&newSignatureBuffer[newSignatureOffset], originalSignature + 1 + oldLocalsLen, copyLength);
    //    newSignatureOffset += copyLength;
    //}

    //// exception 
    //newSignatureBuffer[newSignatureOffset++] = ELEMENT_TYPE_CLASS;
    //memcpy(&newSignatureBuffer[newSignatureOffset], &exceptionTypeRefBuffer, exceptionTypeRefSize);
    //newSignatureOffset += exceptionTypeRefSize;

    //// return value if not void
    //if (!interceptor->info.Signature.ReturnType.IsVoid) {
    //    memcpy(&newSignatureBuffer[newSignatureOffset], &interceptor->info.Signature.ReturnType.Raw[0], returnSignatureTypeSize);
    //    newSignatureOffset += returnSignatureTypeSize;
    //}

    //// Get new locals token
    //mdToken newLocalVarSig;
    //hr = metadataEmit->GetTokenFromSig(newSignatureBuffer, newSignatureSize, &newLocalVarSig);
    //if (FAILED(hr))
    //{
    //    logging::log(logging::LogLevel::_ERROR, "Failed local sig {0}"_W, interceptor->interceptor.Interceptor.AssemblyName);
    //    return hr;
    //}

    //rewriter->SetTkLocalVarSig(newLocalVarSig);

    //ULONG exceptionIndex = newLocalsCount - 1;
    //ULONG returnIndex = -1;
    //if (!interceptor->info.Signature.ReturnType.IsVoid) {
    //    exceptionIndex--;
    //    returnIndex = newLocalsCount - 1;
    //}

    // define interceptor.dll
    mdModuleRef baseDllRef;
    GetWrapperRef(hr, metadataAssemblyEmit, baseDllRef, interceptor->interceptor.Interceptor.AssemblyName);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::_ERROR, "Failed GetWrapperRef {0}"_W, interceptor->interceptor.Interceptor.AssemblyName);
        return hr;
    }

    // define interceptor type
    mdTypeRef interceptorTypeRef;
    hr = metadataEmit->DefineTypeRefByName(
        baseDllRef,
        interceptor->interceptor.Interceptor.TypeName.data(),
        &interceptorTypeRef);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::_ERROR, "Failed DefineTypeRefByName {0}"_W, interceptor->interceptor.Interceptor.TypeName);
    }

    std::vector<BYTE> beforeSignature = {
    IMAGE_CEE_CS_CALLCONV_DEFAULT,
    0,
    ELEMENT_TYPE_VOID };

    // define Before method
    mdMemberRef beforeRef;
    hr = metadataEmit->DefineMemberRef(
        interceptorTypeRef,
        _const::BeforeMethod.data(),
        beforeSignature.data(),
        beforeSignature.size(),
        &beforeRef);

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::_ERROR, "Failed DefineMemberRef {0}"_W, interceptor->interceptor.Interceptor.TypeName);
    }

    // call Before method
    auto beginFirst = helper.CallMember(beforeRef, false);

    auto leaveBeforeTry = helper.LeaveS();
    auto nopBeforeCatch = helper.Pop();
    auto leaveBeforeCatch = helper.LeaveS();

    // try/catch for Interceptor.Begin
    rewriter::EHClause beforeEx{};
    beforeEx.m_Flags = COR_ILEXCEPTION_CLAUSE_NONE;
    beforeEx.m_pTryBegin = beginFirst;
    beforeEx.m_pTryEnd = nopBeforeCatch;
    beforeEx.m_pHandlerBegin = nopBeforeCatch;
    beforeEx.m_pHandlerEnd = leaveBeforeCatch;
    beforeEx.m_ClassToken = exceptionTypeRef;

    leaveBeforeCatch->m_pTarget = helper.GetCurrentInstr();
    leaveBeforeTry->m_pTarget = helper.GetCurrentInstr();

    // new ret instruction
    auto newRet = helper.NewInstr();
    newRet->m_opcode = rewriter::CEE_RET;
    rewriter->InsertAfter(rewriter->GetILList()->m_pPrev, newRet);
    helper.SetILPosition(newRet);

    // main catch
    //auto mainCatch = helper.StLocal(exceptionIndex);
    auto mainCatch = helper.Pop();
    helper.SetILPosition(newRet);
    auto rethrow = helper.Rethrow();
    auto nopFinally = helper.Nop();

    // finally

    std::vector<BYTE> afterSignature = {
    IMAGE_CEE_CS_CALLCONV_DEFAULT,
    0,
    ELEMENT_TYPE_VOID };

    // define After method
    mdMemberRef afterRef;
    hr = metadataEmit->DefineMemberRef(
        interceptorTypeRef,
        _const::AfterMethod.data(),
        afterSignature.data(),
        afterSignature.size(),
        &beforeRef);

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::_ERROR, "Failed DefineMemberRef {0}"_W, interceptor->interceptor.Interceptor.TypeName);
    }

    // call After method
    auto afterFirst = helper.CallMember(beforeRef, false);

    auto leaveAfterTry = helper.LeaveS();
    auto nopAfterCatch = helper.Pop();
    auto leaveAfterCatch = helper.LeaveS();

    // try/catch for Interceptor.Begin
    rewriter::EHClause afterEx{};
    afterEx.m_Flags = COR_ILEXCEPTION_CLAUSE_NONE;
    afterEx.m_pTryBegin = afterFirst;
    afterEx.m_pTryEnd = nopAfterCatch;
    afterEx.m_pHandlerBegin = nopAfterCatch;
    afterEx.m_pHandlerEnd = leaveAfterCatch;
    afterEx.m_ClassToken = exceptionTypeRef;

    // finally

    auto endFinally = helper.EndFinally();
    leaveAfterCatch->m_pTarget = endFinally;
    leaveAfterTry->m_pTarget = endFinally;

    /*if (!interceptor->info.Signature.ReturnType.IsVoid) {
        helper.LoadLocal(returnIndex);
    }*/

    // ret -> leave_s
    for (auto pInstr = rewriter->GetILList()->m_pNext; pInstr != rewriter->GetILList(); pInstr = pInstr->m_pNext)
    {
        switch (pInstr->m_opcode)
        {
        case rewriter::CEE_RET:
        {
            if (pInstr != newRet)
            {
                if (interceptor->info.Signature.ReturnType.IsVoid)
                {
                    pInstr->m_opcode = rewriter::CEE_LEAVE_S;
                    pInstr->m_pTarget = endFinally->m_pNext;
                }
                else
                {
                    /*pInstr->m_opcode = rewriter::CEE_STLOC;
                    pInstr->m_Arg16 = static_cast<INT16>(returnIndex);

                    auto leaveInstr = rewriter->NewILInstr();
                    leaveInstr->m_opcode = rewriter::CEE_LEAVE_S;
                    leaveInstr->m_pTarget = endFinally->m_pNext;
                    rewriter->InsertAfter(pInstr, leaveInstr);*/
                }
            }
            break;
        }
        default:
            break;
        }
    }

    rewriter::EHClause exClause{};
    exClause.m_Flags = COR_ILEXCEPTION_CLAUSE_NONE;
    exClause.m_pTryBegin = beginFirst;
    exClause.m_pTryEnd = mainCatch;
    exClause.m_pHandlerBegin = mainCatch;
    exClause.m_pHandlerEnd = rethrow;
    exClause.m_ClassToken = exceptionTypeRef;

    rewriter::EHClause finallyClause{};
    finallyClause.m_Flags = COR_ILEXCEPTION_CLAUSE_FINALLY;
    finallyClause.m_pTryBegin = beginFirst;
    finallyClause.m_pTryEnd = rethrow->m_pNext;
    finallyClause.m_pHandlerBegin = nopFinally;// rethrow->m_pNext;
    finallyClause.m_pHandlerEnd = endFinally;

    auto ehCount = rewriter->GetEHCount();
    auto ehPointer = rewriter->GetEHPointer();

    auto newEx = new rewriter::EHClause[ehCount + 4];
    for (unsigned i = 0; i < ehCount; i++)
    {
        newEx[i] = ehPointer[i];
    }

    ehCount += 4;
    newEx[ehCount - 4] = beforeEx;
    newEx[ehCount - 3] = afterEx;
    newEx[ehCount - 2] = exClause;
    newEx[ehCount - 1] = finallyClause;
    rewriter->SetEHClause(newEx, ehCount);

    logging::log(logging::LogLevel::VERBOSE, "{0}"_W, profiler->ilDumper.DumpILCodes("rejit after ", rewriter, interceptor->info, metadataImport));
    std::cout << util::ToString(profiler->ilDumper.DumpILCodes("rejit after ", rewriter, interceptor->info, metadataImport)) << std::endl;

    hr = rewriter->Export();

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::_ERROR, "GetReJITParameters rewriter->Export failed"_W);
        return S_FALSE;
    }

    logging::log(logging::LogLevel::INFO, "GetReJITParameters done"_W);

    return S_OK;
}