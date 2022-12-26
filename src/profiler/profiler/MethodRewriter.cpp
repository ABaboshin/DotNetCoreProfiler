#include <algorithm>
#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>
#include "MethodRewriter.h"
#include "logging/logging.h"
#include "CorProfiler.h"
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
        logging::log(logging::LogLevel::PROFILERERROR, "GetReJITParameters rewriter->Import failed"_W);
        return hr;
    }

    rewriter::ILRewriterHelper helper(rewriter);
    helper.SetILPosition(rewriter->GetILList()->m_pNext);

    //auto firstIL = helper.GetCurrentInstr();

    if (profiler->loadedModules.find(interceptor->interceptor.Interceptor.AssemblyName) == profiler->loadedModules.end())
    {
        logging::log(logging::LogLevel::VERBOSE, "Skip rejit because interceptor assembly is not loaded yet"_W);
        return hr;
    }
    else
    {
        logging::log(logging::LogLevel::VERBOSE, "Continue rejit as interceptor assembly is loaded {0}"_W, profiler->loadedModules[interceptor->interceptor.Interceptor.AssemblyName]);
    }

    ComPtr<IUnknown> metadataInterfaces;
    hr = profiler->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf());
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::PROFILERERROR, "Failed GetModuleMetaData {0}"_W, interceptor->interceptor.Interceptor.AssemblyName);
        return hr;
    }

    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);
    auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);
    auto assemblyImport = metadataInterfaces.As<IMetaDataAssemblyImport>(IID_IMetaDataEmit);
    auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);

    logging::log(logging::LogLevel::VERBOSE, "{0}"_W, profiler->ilDumper.DumpILCodes("rejit before ", rewriter, interceptor->info, metadataImport));

    std::cout << util::ToString(profiler->ilDumper.DumpILCodes("rejit before ", rewriter, interceptor->info, metadataImport)) << std::endl;

    // define mscorlib.dll
    mdModuleRef mscorlibRef;
    hr = GetMsCorLibRef(metadataAssemblyEmit, mscorlibRef);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::PROFILERERROR, "Failed GetMsCorLibRef {0}"_W, interceptor->interceptor.Interceptor.AssemblyName);
        return hr;
    }

    // Define System.Exception
    mdTypeRef exceptionTypeRef;
    metadataEmit->DefineTypeRefByName(mscorlibRef, _const::SystemException.data(), &exceptionTypeRef);

    // define interceptor.dll
    mdModuleRef baseDllRef;
    hr = GetWrapperRef(metadataAssemblyEmit, baseDllRef, interceptor->interceptor.Interceptor.AssemblyName);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::PROFILERERROR, "Failed GetWrapperRef {0}"_W, interceptor->interceptor.Interceptor.AssemblyName);
        return hr;
    }

    ULONG exceptionIndex = 0;
    ULONG returnIndex = 0;

    hr = DefineLocalSignature(rewriter, moduleId, exceptionTypeRef, *interceptor, &exceptionIndex, &returnIndex);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::PROFILERERROR, "Failed DefineLocalSignature {0}"_W, interceptor->interceptor.Interceptor.AssemblyName);
        return hr;
    }

    // initialize local variables
    hr = InitLocalValues(helper, rewriter, moduleId, *interceptor, exceptionIndex, returnIndex, baseDllRef);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::PROFILERERROR, "Failed InitLocalValues {0}"_W, interceptor->interceptor.Interceptor.AssemblyName);
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
        logging::log(logging::LogLevel::PROFILERERROR, "Failed DefineTypeRefByName {0}"_W, interceptor->interceptor.Interceptor.TypeName);
    }

    interceptor->info.Signature.ParseArguments();

    // call Before method
    rewriter::ILInstr* beginFirst;
    hr = CreateBeforeMethod(helper, &beginFirst, metadataEmit, metadataAssemblyEmit, interceptorTypeRef, *interceptor);
    if (FAILED(hr)) {
        logging::log(logging::LogLevel::PROFILERERROR, "Failed CreateBeforeMethod");
        return hr;
    }

    auto leaveBeforeTry = helper.LeaveS();
    // TODO log exception
    auto nopBeforeCatch = helper.Pop();
    auto leaveBeforeCatch = helper.LeaveS();

    // try/catch for Interceptor.Before
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
    auto mainCatch = helper.StLocal(exceptionIndex);
    auto mainCatchLeave = helper.Rethrow(); // helper.LeaveS();
    mainCatchLeave->m_pTarget = newRet;
    helper.SetILPosition(newRet);
    //auto rethrow = helper.StLocal(exceptionIndex);
    auto nopFinally = helper.Nop();

    // finally

    // call After method
    rewriter::ILInstr* afterFirst;
    hr = CreateAfterMethod(helper, &afterFirst, metadataEmit, interceptorTypeRef, *interceptor);
    if (FAILED(hr)) {
        logging::log(logging::LogLevel::PROFILERERROR, "Failed CreateAfterMethod");
        return hr;
    }

    auto leaveAfterTry = helper.LeaveS();
    // TODO log exception
    auto nopAfterCatch = helper.Pop();
    auto leaveAfterCatch = helper.LeaveS();

    // try/catch for Interceptor.After
    rewriter::EHClause afterEx{};
    afterEx.m_Flags = COR_ILEXCEPTION_CLAUSE_NONE;
    afterEx.m_pTryBegin = afterFirst;
    afterEx.m_pTryEnd = nopAfterCatch;
    afterEx.m_pHandlerBegin = nopAfterCatch;
    afterEx.m_pHandlerEnd = leaveAfterCatch;
    afterEx.m_ClassToken = exceptionTypeRef;

    // finally

    // if ex != null throw ex
    /*auto ldLocal = helper.LoadLocal(exceptionIndex);
    auto brFalseS = helper.BrFalseS();
    helper.LoadLocal(exceptionIndex);
    helper.Throw();*/

    auto endFinally = helper.EndFinally();
    leaveAfterCatch->m_pTarget = endFinally;// ldLocal;
    leaveAfterTry->m_pTarget = endFinally;
    //brFalseS->m_pTarget = endFinally;

    auto isVoid = interceptor->info.Signature.ReturnType.IsVoid;
    //isVoid = true;

    if (!isVoid) {
        helper.LoadLocal(returnIndex);
    }

    // ret -> leave_s
    for (auto pInstr = rewriter->GetILList()->m_pNext; pInstr != rewriter->GetILList(); pInstr = pInstr->m_pNext)
    {
        switch (pInstr->m_opcode)
        {
        case rewriter::CEE_RET:
        {
            if (pInstr != newRet)
            {
                if (isVoid)
                {
                    pInstr->m_opcode = rewriter::CEE_LEAVE_S;
                    pInstr->m_pTarget = endFinally->m_pNext;
                }
                else
                {
                    pInstr->m_opcode = rewriter::CEE_STLOC;
                    pInstr->m_Arg16 = static_cast<INT16>(returnIndex);

                    auto leaveInstr = rewriter->NewILInstr();
                    leaveInstr->m_opcode = rewriter::CEE_LEAVE_S;
                    leaveInstr->m_pTarget = endFinally->m_pNext;
                    rewriter->InsertAfter(pInstr, leaveInstr);
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
    exClause.m_pHandlerEnd = mainCatchLeave;
    exClause.m_ClassToken = exceptionTypeRef;

    rewriter::EHClause finallyClause{};
    finallyClause.m_Flags = COR_ILEXCEPTION_CLAUSE_FINALLY;
    finallyClause.m_pTryBegin = beginFirst;
    finallyClause.m_pTryEnd = mainCatchLeave->m_pNext;
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
        logging::log(logging::LogLevel::PROFILERERROR, "GetReJITParameters rewriter->Export failed"_W);
        return hr;
    }

    logging::log(logging::LogLevel::INFO, "GetReJITParameters done"_W);

    return S_OK;
}

// TODO
HRESULT MethodRewriter::CreateBeforeMethod(rewriter::ILRewriterHelper& helper, rewriter::ILInstr** instr, util::ComPtr< IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdTypeRef interceptorTypeRef, const RejitInfo& interceptor)
{
    HRESULT hr;

    auto interceptorNumberOfArguments = interceptor.info.Signature.NumberOfArguments() + 1;

    // define generic Before method
    std::vector<BYTE> genericBeforeSignature = {
    IMAGE_CEE_CS_CALLCONV_GENERIC,
        (BYTE)interceptorNumberOfArguments,
        (BYTE)interceptorNumberOfArguments,
        ELEMENT_TYPE_VOID,
        
         };
    for (auto i = 0; i < interceptorNumberOfArguments; i++)
    {
        //genericBeforeSignature.push_back(ELEMENT_TYPE_BYREF);
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

    std::cout << "generic sig" << std::endl;
    for (auto i = 0; i < genericBeforeSignature.size(); i++)
    {
        std::cout << std::hex << (int)genericBeforeSignature[i] << std::endl;
    }

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::PROFILERERROR, "Failed CreateBeforeMethod {0}"_W, interceptor.interceptor.Interceptor.TypeName);
        return hr;
    }
    
    // get target type id

    auto isValueType = false;
    mdToken targetTypeRef = mdTokenNil;
    if (interceptor.info.Signature.IsInstanceMethod())
    {
        hr = GetTargetTypeRef(interceptor.info.Type, metadataEmit, metadataAssemblyEmit, &targetTypeRef, &isValueType);
        if (FAILED(hr)) {
            logging::log(logging::LogLevel::PROFILERERROR, "Failed CreateBeforeMethod {0}"_W, interceptor.interceptor.Interceptor.TypeName);
            return hr;
        }
    }

    // load this or null
    if (targetTypeRef == mdTokenNil || !interceptor.info.Signature.IsInstanceMethod()) {
        std::cout << "targetTypeRef " << targetTypeRef << std::endl;
        std::cout << "IsInstanceMethod " << interceptor.info.Signature.IsInstanceMethod() << std::endl;
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
        helper.LoadArgument(i + (interceptor.info.Signature.IsInstanceMethod() ? 1 : 0));
    }

    // non generic Before method
    if (targetTypeRef == mdTokenNil) {
        hr = GetObjectTypeRef(metadataEmit, metadataAssemblyEmit, &targetTypeRef);
        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::PROFILERERROR, "Failed CreateBeforeMethod GetObjectTypeRef"_W);
            return hr;
        }
    }

    unsigned targetTypeBuffer = 0;
    ULONG targetTypeSize = CorSigCompressToken(targetTypeRef, &targetTypeBuffer);

    //auto signatureLength = 4 + targetTypeSize;
    std::cout << "num args " << interceptor.info.Signature.NumberOfArguments() << std::endl;
    
    //for (auto i = 0; i < interceptor.info.Signature.NumberOfArguments(); i++)
    //{
    //    signatureLength += interceptor.info.Signature.Arguments[i].Raw.size();
    //}

    COR_SIGNATURE signature[1024];
    unsigned offset = 0;

    signature[offset++] = IMAGE_CEE_CS_CALLCONV_GENERICINST;
    //signature[offset++] = interceptorNumberOfArguments;
    signature[offset++] = interceptorNumberOfArguments;
    //signature[offset++] = ELEMENT_TYPE_VOID;

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
        memcpy(&signature[offset], &interceptor.info.Signature.Arguments[i].Raw[0], interceptor.info.Signature.Arguments[i].Raw.size());
        offset += interceptor.info.Signature.Arguments[i].Raw.size();
    }

    std::cout << "non generic sig" << std::endl;
    for (auto i = 0; i < offset; i++)
    {
        std::cout << std::hex << (int)signature[i] << std::endl;
    }

    std::cout << "offset " << offset << /*" length " << signatureLength <<*/ std::endl;
    
    mdMethodSpec beforeMethodSpec = mdMethodSpecNil;
    hr = metadataEmit->DefineMethodSpec(genericBeforeRef, signature,
        offset, &beforeMethodSpec);

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::PROFILERERROR, "Failed CreateBeforeMethod DefineMethodSpec"_W);
        return hr;
    }

    helper.CallMember(beforeMethodSpec, false);
    //*instr = helper.Nop();
    return S_OK;

}

HRESULT MethodRewriter::GetTargetTypeRef(const info::TypeInfo& targetType, util::ComPtr< IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdToken* targetTypeRef, bool* isValueType) {
    HRESULT hr;
    *isValueType = targetType.IsValueType;
    *targetTypeRef = targetType.TypeSpec;

    if (*targetTypeRef != mdTypeSpecNil) {
        return S_OK;
    }

    // TODO cache references to known types
    mdTypeRef objectTypeRef;
    hr = GetObjectTypeRef(metadataEmit, metadataAssemblyEmit, &objectTypeRef);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::PROFILERERROR, "Failed GetTargetTypeRef GetObjectTypeRef"_W);
        return hr;
    }

    if (targetType.ParentTypeInfo != nullptr) {
        bool parentTypeIsGeneric = false;
        auto parentType = targetType.ParentTypeInfo.get();
        while (true) {
            if (parentType->IsGenericClassRef) {
                parentTypeIsGeneric = true;
                break;
            }

            if (parentType->ParentTypeInfo == nullptr) {
                break;
            }

            parentType = parentType->ParentTypeInfo.get();
        }

        // if parent type is generic
        if (parentTypeIsGeneric) {
            // and the target type is a struct => cannot be instrumentialized
            if (targetType.IsValueType) {
                *targetTypeRef = mdTokenNil;
                return S_OK;
            }

            // otherwise use system.object
            *targetTypeRef = objectTypeRef;

            return S_OK;
        }
    }

    auto currentType = &targetType;
    // if the target type is a generic one
    // lookup for the nearest non generic parent
    while (currentType != nullptr && targetType.IsGenericClassRef) {
        currentType = currentType->ParentTypeInfo.get();
    }

    if (currentType == nullptr)
    {
        *targetTypeRef = objectTypeRef;
        return S_OK;
    }

    *isValueType = currentType->IsValueType;
    *targetTypeRef = currentType->Id;

    return S_OK;
}

//TODO
HRESULT MethodRewriter::CreateAfterMethod(rewriter::ILRewriterHelper& helper, rewriter::ILInstr** instr, util::ComPtr< IMetaDataEmit2>& metadataEmit, mdTypeRef interceptorTypeRef, const RejitInfo& interceptor)
{
    *instr = helper.Nop();
    return S_OK;
    HRESULT hr;
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
        &afterRef);

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::PROFILERERROR, "Failed CreateAfterMethod {0}"_W, interceptor.interceptor.Interceptor.TypeName);
        return hr;
    }

    *instr = helper.CallMember(afterRef, false);
    return S_OK;
}

HRESULT MethodRewriter::DefineLocalSignature(rewriter::ILRewriter* rewriter, ModuleID moduleId, mdTypeRef exceptionTypeRef, const RejitInfo& interceptor, ULONG* exceptionIndex, ULONG* returnIndex) {
    HRESULT hr;

    ComPtr<IUnknown> metadataInterfaces;
    hr = profiler->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf());
    if (FAILED(hr)) {
        logging::log(logging::LogLevel::PROFILERERROR, "Failed GetModuleMetaData GetSigFromToken");
        return hr;
    }
    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);
    auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);

    // modify local signature
    // to add an exception and the return value
    mdToken localVarSig = rewriter->GetTkLocalVarSig();
    PCCOR_SIGNATURE originalSignature = nullptr;
    ULONG originalSignatureSize = 0;

    if (localVarSig != mdTokenNil) {
        hr = metadataImport->GetSigFromToken(localVarSig, &originalSignature, &originalSignatureSize);
        if (FAILED(hr)) {
            logging::log(logging::LogLevel::PROFILERERROR, "Failed DefineLocalSignature GetSigFromToken");
            return hr;
        }
    }

    // exception type buffer and size
    unsigned exceptionTypeRefBuffer;
    auto exceptionTypeRefSize = CorSigCompressToken(exceptionTypeRef, &exceptionTypeRefBuffer);

    // return type signature
    ULONG returnSignatureTypeSize = 0;
    mdTypeSpec returnValueTypeSpec = mdTypeSpecNil;

    auto newSignatureSize = originalSignatureSize + (1 + exceptionTypeRefSize);
    ULONG newSignatureOffset = 0;
    ULONG newLocalsCount = 1;
    
    auto returnSignature = interceptor.info.Signature.ReturnType.Raw;
    auto isVoid = interceptor.info.Signature.ReturnType.IsVoid;

    if (!isVoid)
    {
        returnSignatureTypeSize = returnSignature.size();

        mdTypeSpec returnValueTypeSpec = mdTypeSpecNil;
        hr = metadataEmit->GetTokenFromTypeSpec(&returnSignature[0], returnSignature.size(), &returnValueTypeSpec);
        newSignatureSize += (returnSignatureTypeSize);
        newLocalsCount++;
    }

    ULONG oldLocalsBuffer;
    ULONG oldLocalsLen = 0;
    unsigned newLocalsBuffer;
    ULONG newLocalsLen;

    if (originalSignatureSize == 0)
    {
        newSignatureSize += 2;
        newLocalsLen = CorSigCompressData(newLocalsCount, &newLocalsBuffer);
    }
    else
    {
        oldLocalsLen = CorSigUncompressData(originalSignature + 1, &oldLocalsBuffer);
        newLocalsCount += oldLocalsBuffer;
        newLocalsLen = CorSigCompressData(newLocalsCount, &newLocalsBuffer);
        newSignatureSize += newLocalsLen - oldLocalsLen;
    }

    // New signature declaration
    COR_SIGNATURE newSignatureBuffer[500];
    newSignatureBuffer[newSignatureOffset++] = IMAGE_CEE_CS_CALLCONV_LOCAL_SIG;

    // Set the locals count
    memcpy(&newSignatureBuffer[newSignatureOffset], &newLocalsBuffer, newLocalsLen);
    newSignatureOffset += newLocalsLen;

    if (originalSignatureSize > 0)
    {
        const auto copyLength = originalSignatureSize - 1 - oldLocalsLen;
        memcpy(&newSignatureBuffer[newSignatureOffset], originalSignature + 1 + oldLocalsLen, copyLength);
        newSignatureOffset += copyLength;
    }

    // exception 
    newSignatureBuffer[newSignatureOffset++] = ELEMENT_TYPE_CLASS;
    memcpy(&newSignatureBuffer[newSignatureOffset], &exceptionTypeRefBuffer, exceptionTypeRefSize);
    newSignatureOffset += exceptionTypeRefSize;

    std::cout << " exceptionTypeRefSize " << exceptionTypeRefSize << std::endl;

    // return value if not void
    if (!isVoid) {
        memcpy(&newSignatureBuffer[newSignatureOffset], &returnSignature[0], returnSignatureTypeSize);
        std::cout << " returnSignatureTypeSize " << returnSignatureTypeSize << std::endl;
        newSignatureOffset += returnSignatureTypeSize;
    }

    // Get new locals token
    mdToken newLocalVarSig;
    hr = metadataEmit->GetTokenFromSig(newSignatureBuffer, newSignatureSize, &newLocalVarSig);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::PROFILERERROR, "Failed local sig {0}"_W, interceptor.interceptor.Interceptor.AssemblyName);
        return hr;
    }

    rewriter->SetTkLocalVarSig(newLocalVarSig);

    *exceptionIndex = newLocalsCount - 1;
    *returnIndex = -1;
    if (!isVoid) {
        (*exceptionIndex)--;
        *returnIndex = newLocalsCount - 1;
    }

    return S_OK;
}

