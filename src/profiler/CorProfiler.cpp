// Copyright (c) .NET Foundation and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <algorithm>
#include <iostream>
#include <string>
#include "CorProfiler.h"
#include "corhlpr.h"
#include "corhdr.h"
#include "ILRewriter.h"
#include "helpers.h"
#include "util.h"
#include "ComPtr.h"
#include "ILRewriterHelper.h"


CorProfiler* profiler = nullptr;

static void STDMETHODCALLTYPE Enter(FunctionID functionId)
{
    std::cout << "Enter " << functionId << std::endl;
    std::cout << std::flush;
}

static void STDMETHODCALLTYPE Leave(FunctionID functionId)
{
    std::cout << "Leave " << functionId << std::endl;
    std::cout << std::flush;
}

COR_SIGNATURE enterLeaveMethodSignature[] = { IMAGE_CEE_CS_CALLCONV_STDCALL, 0x01, ELEMENT_TYPE_VOID, ELEMENT_TYPE_I };

void(STDMETHODCALLTYPE* EnterMethodAddress)(FunctionID) = &Enter;
void(STDMETHODCALLTYPE* LeaveMethodAddress)(FunctionID) = &Leave;

CorProfiler::CorProfiler() : refCount(0), corProfilerInfo(nullptr)
{
}

CorProfiler::~CorProfiler()
{
    if (this->corProfilerInfo != nullptr)
    {
        this->corProfilerInfo->Release();
        this->corProfilerInfo = nullptr;
    }
}

HRESULT STDMETHODCALLTYPE CorProfiler::Initialize(IUnknown* pICorProfilerInfoUnk)
{
    HRESULT queryInterfaceResult = pICorProfilerInfoUnk->QueryInterface(__uuidof(ICorProfilerInfo8), reinterpret_cast<void**>(&this->corProfilerInfo));

    if (FAILED(queryInterfaceResult))
    {
        return E_FAIL;
    }

    DWORD eventMask = COR_PRF_MONITOR_JIT_COMPILATION |
        COR_PRF_DISABLE_TRANSPARENCY_CHECKS_UNDER_FULL_TRUST | /* helps the case where this profiler is used on Full CLR */
        COR_PRF_DISABLE_INLINING;

    auto hr = this->corProfilerInfo->SetEventMask(eventMask);

    is_attached = true;
    profiler = this;

    moduleNames = Split(GetEnvironmentValue("PROFILER_ENABLEDMODULES"_W), L',');
    modules = std::vector<ModuleID>();

    for (auto&& module : moduleNames) {
        std::cout << "found " << ToString(module) << std::endl;
    }

    wrapperDllPath = GetEnvironmentValue("PROFILER_WRAPPER_DLL"_W);
    wrapperType = GetEnvironmentValue("PROFILER_WRAPPER_TYPE"_W);
    wrapperAssemblyName = GetEnvironmentValue("PROFILER_WRAPPER_ASSEMBLY"_W);

    std::cout << "wrapperDll " << ToString(wrapperDllPath) << std::endl;
    std::cout << "wrapperType " << ToString(wrapperType) << std::endl;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::Shutdown()
{
    if (this->corProfilerInfo != nullptr)
    {
        this->corProfilerInfo->Release();
        this->corProfilerInfo = nullptr;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AppDomainCreationStarted(AppDomainID appDomainId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AppDomainShutdownStarted(AppDomainID appDomainId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AssemblyLoadStarted(AssemblyID assemblyId)
{
    std::cout << "AssemblyLoadStarted " << assemblyId << std::endl;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus)
{
    std::cout << "AssemblyLoadFinished " << assemblyId << std::endl;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AssemblyUnloadStarted(AssemblyID assemblyId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ModuleLoadStarted(ModuleID moduleId)
{
    std::cout << "ModuleLoadStarted " << moduleId << std::endl;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ModuleUnloadStarted(ModuleID moduleId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID AssemblyId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ClassLoadStarted(ClassID classId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ClassLoadFinished(ClassID classId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ClassUnloadStarted(ClassID classId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ClassUnloadFinished(ClassID classId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::FunctionUnloadStarted(FunctionID functionId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock)
{
    HRESULT hr;
    mdToken functionToken;
    ClassID classId;
    ModuleID moduleId;

    IfFailRet(this->corProfilerInfo->GetFunctionInfo(functionId, &classId, &moduleId, &functionToken));

    const ModuleInfo moduleInfo = GetModuleInfo(this->corProfilerInfo, moduleId);
    if (std::find(moduleNames.begin(), moduleNames.end(), moduleInfo.assembly.name) == moduleNames.end()) {
        return S_OK;
    }

    //std::cout << "JITCompilationStarted " << functionId << std::endl;

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    const auto metadataEmit =
        metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);

    mdSignature enterLeaveMethodSignatureToken;
    metadataEmit->GetTokenFromSig(enterLeaveMethodSignature, sizeof(enterLeaveMethodSignature), &enterLeaveMethodSignatureToken);

    const auto assemblyEmit =
        metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);

    const auto metadataImport =
        metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    auto functionInfo = GetFunctionInfo(metadataImport, functionToken);

    hr = functionInfo.signature.TryParse();
    IfFailRet(hr);

    //std::cout << ToString(functionInfo.type.name) << "." << ToString(functionInfo.name) << " " << functionInfo.signature.NumberOfArguments() << std::endl << std::flush;

    // it's a hack, find a better way to figure out the entrypoint
    // i.e. catch the first call in an AppDomain
    if (ends_with(functionInfo.name, "Main"_W))
    {
        std::cout << "ENTRYPOINT!!" << std::endl;

        return LoadAssemblyBefore(this->corProfilerInfo,
            metadataImport,
            metadataEmit,
            assemblyEmit,
            nullptr,
            moduleId,
            functionToken,
            functionId,
            wrapperDllPath,
            enterLeaveMethodSignatureToken);
    }

    // define reference to a wrapper .net dll
    ASSEMBLYMETADATA wrapperAssemblyMetadata{};
    wrapperAssemblyMetadata.usMajorVersion = 1;
    wrapperAssemblyMetadata.usMinorVersion = 0;
    wrapperAssemblyMetadata.usBuildNumber = 0;
    wrapperAssemblyMetadata.usRevisionNumber = 0;

    mdModuleRef wrapperAssemblyRef;
    hr = CreateAssemblyRef(assemblyEmit, &wrapperAssemblyRef, std::vector<BYTE>(), wrapperAssemblyMetadata, wrapperAssemblyName);

    // define wrapper type
    mdTypeRef methodTraceTypeRef;
    hr = metadataEmit->DefineTypeRefByName(
        wrapperAssemblyRef,
        wrapperType.data(),
        &methodTraceTypeRef);
    IfFailRet(hr);

    // define test
    COR_SIGNATURE testSig[] =
    {
        IMAGE_CEE_CS_CALLCONV_DEFAULT,
        0,
        ELEMENT_TYPE_VOID,
    };

    mdMemberRef testMemberRef;
    hr = metadataEmit->DefineMemberRef(
        wrapperAssemblyRef,
        ".ctor"_W.data(),
        testSig,
        sizeof(testSig),
        &testMemberRef);
    IfFailRet(hr);

    // define .ctor
    COR_SIGNATURE ctorSig[] =
    {
        IMAGE_CEE_CS_CALLCONV_HASTHIS ,
        0,
        ELEMENT_TYPE_VOID,
    };

    mdMemberRef ctorMemberRef;
    hr = metadataEmit->DefineMemberRef(
        wrapperAssemblyRef,
        ".ctor"_W.data(),
        ctorSig,
        sizeof(ctorSig),
        &ctorMemberRef);
    IfFailRet(hr);

    // define start method
    COR_SIGNATURE startSig[] =
    {
        IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_HASTHIS ,
        0x02,
        ELEMENT_TYPE_VOID,
    };

    mdMemberRef startMemberRef;
    hr = metadataEmit->DefineMemberRef(
        wrapperAssemblyRef,
        "Start"_W.data(),
        startSig,
        sizeof(startSig),
        &startMemberRef);
    IfFailRet(hr);

    // define finish method
    COR_SIGNATURE finishSig[] =
    {
        IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_HASTHIS,
        0x02,
        ELEMENT_TYPE_VOID,
        ELEMENT_TYPE_OBJECT,
        ELEMENT_TYPE_OBJECT
    };
    mdMemberRef finishMemberRef;
    hr = metadataEmit->DefineMemberRef(
        methodTraceTypeRef,
        "Finish"_W.data(),
        finishSig,
        sizeof(finishSig),
        &finishMemberRef);
    IfFailRet(hr);

    // define mscorlib.dll
    ASSEMBLYMETADATA metadata{};
    metadata.usMajorVersion = 4;
    metadata.usMinorVersion = 0;
    metadata.usBuildNumber = 0;
    metadata.usRevisionNumber = 0;

    mdModuleRef mscorlibRef;
    hr = CreateAssemblyRef(assemblyEmit, &mscorlibRef, std::vector<BYTE>{ 0xB7, 0x7A, 0x5C, 0x56, 0x19, 0x34, 0xE0, 0x89 }, metadata, "mscorlib"_W);
    IfFailRet(hr);

    // define System.Exception
    mdTypeRef exTypeRef;
    hr = metadataEmit->DefineTypeRefByName(
        mscorlibRef,
        "System.Exception"_W.data(),
        &exTypeRef);
    IfFailRet(hr);

    // define System.Type
    mdTypeRef typeRef;
    hr = metadataEmit->DefineTypeRefByName(
        mscorlibRef,
        "System.Type"_W.data(),
        &typeRef);
    IfFailRet(hr);

    // define System.RuntimeTypeHandle
    mdTypeRef runtimeTypeHandleRef;
    hr = metadataEmit->DefineTypeRefByName(
        mscorlibRef,
        "System.RuntimeTypeHandle"_W.data(),
        &runtimeTypeHandleRef);
    IfFailRet(hr);

    // define System.Type.GetTypeFromHandle
    unsigned runtimeTypeHandle_buffer;
    unsigned type_buffer;
    auto runtimeTypeHandle_size = CorSigCompressToken(runtimeTypeHandleRef, &runtimeTypeHandle_buffer);
    auto type_size = CorSigCompressToken(typeRef, &type_buffer);
    auto* getTypeFromHandleSig = new COR_SIGNATURE[runtimeTypeHandle_size + type_size + 4];
    unsigned offset = 0;
    getTypeFromHandleSig[offset++] = IMAGE_CEE_CS_CALLCONV_DEFAULT;
    getTypeFromHandleSig[offset++] = 0x01;
    getTypeFromHandleSig[offset++] = ELEMENT_TYPE_CLASS;
    memcpy(&getTypeFromHandleSig[offset], &type_buffer, type_size);
    offset += type_size;
    getTypeFromHandleSig[offset++] = ELEMENT_TYPE_VALUETYPE;
    memcpy(&getTypeFromHandleSig[offset], &runtimeTypeHandle_buffer, runtimeTypeHandle_size);
    offset += runtimeTypeHandle_size;

    mdToken getTypeFromHandleToken;
    hr = metadataEmit->DefineMemberRef(
        typeRef,
        "GetTypeFromHandle"_W.data(),
        getTypeFromHandleSig,
        sizeof(getTypeFromHandleSig),
        &getTypeFromHandleToken);
    IfFailRet(hr);

    // and then rewrite
    ILRewriter rewriter(corProfilerInfo, NULL, moduleId, functionToken);
    IfFailRet(rewriter.Import());

    //ModifyLocalSig
    hr = ModifyLocalSig(metadataImport, metadataEmit, rewriter, exTypeRef, methodTraceTypeRef);
    IfFailRet(hr);

    //define System.Object
    mdTypeRef objectTypeRef;
    hr = metadataEmit->DefineTypeRefByName(
        mscorlibRef,
        "System.Object"_W.data(),
        &objectTypeRef);
    IfFailRet(hr);

    // get return type
    unsigned elementType;
    auto retTypeFlags = functionInfo.signature.GetRet().GetTypeFlags(elementType);

    auto indexRet = rewriter.cNewLocals - 3;
    auto indexEx = rewriter.cNewLocals - 2;
    auto indexMethodTrace = rewriter.cNewLocals - 1;

    ILRewriterHelper rewriteHelper(&rewriter);
    ILInstr* pFirstInstr = rewriter.GetILList()->m_pNext;
    rewriteHelper.SetILPosition(pFirstInstr);

    // define helper type
    mdTypeRef helperTypeRef;
    hr = metadataEmit->DefineTypeRefByName(
        wrapperAssemblyRef,
        "Wrapper.Helper"_W.data(),
        &helperTypeRef);
    IfFailRet(hr);

    // define helper getinstance
    COR_SIGNATURE getInstanceSig[] =
    {
        IMAGE_CEE_CS_CALLCONV_DEFAULT,
        0x00,
        ELEMENT_TYPE_OBJECT
    };
    mdMemberRef getInstanceMemberRef;
    hr = metadataEmit->DefineMemberRef(
        helperTypeRef,
        "GetInstance"_W.data(),
        getInstanceSig,
        sizeof(getInstanceSig),
        &getInstanceMemberRef);
    IfFailRet(hr);

    // define helper getinstance
    COR_SIGNATURE beforeMethodSig[] =
    {
        IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_HASTHIS ,
        0x04,
        ELEMENT_TYPE_OBJECT,
        ELEMENT_TYPE_OBJECT,
        ELEMENT_TYPE_OBJECT,
        ELEMENT_TYPE_SZARRAY,
        ELEMENT_TYPE_OBJECT,
        ELEMENT_TYPE_U4
    };
    mdMemberRef beforeMethodMemberRef;
    hr = metadataEmit->DefineMemberRef(
        helperTypeRef,
        "BeforeMethod"_W.data(),
        beforeMethodSig,
        sizeof(beforeMethodSig),
        &beforeMethodMemberRef);
    IfFailRet(hr);

    std::cout << "try " << ToString(functionInfo.type.name) << "." << ToString(functionInfo.name) << " " << functionInfo.signature.NumberOfArguments() << std::endl << std::flush;

    if (functionInfo.signature.NumberOfArguments() != 0)
    {
        return S_OK;
    }

    std::cout << "rewrite " << ToString(functionInfo.type.name) << "." << ToString(functionInfo.name) << " " << functionInfo.signature.NumberOfArguments() << std::endl << std::flush;

    //rewriteHelper.Nop();
    ////rewriteHelper.Pop();
    //rewriteHelper.CallMember(testMemberRef, false);
    //rewriteHelper.Ret();
    //auto original_methodcall_opcode = pFirstInstr->m_opcode;
    //pFirstInstr->m_opcode = CEE_NOP;
    //ILInstr* pPrevInstr = pFirstInstr;
    //ILInstr* pNewInstr;
    ///*pNewInstr = rewriter.NewILInstr();
    //pNewInstr->m_opcode = CEE_CALL;
    //pNewInstr->m_Arg32 = testMemberRef;
    //rewriter.InsertAfter(pPrevInstr, pNewInstr);
    //pPrevInstr = pNewInstr;*/
    //pNewInstr = rewriter.NewILInstr();
    //pNewInstr->m_opcode = CEE_NOP;
    //rewriter.InsertAfter(pPrevInstr, pNewInstr);
    //pPrevInstr = pNewInstr;
    //pNewInstr = rewriter.NewILInstr();
    //pNewInstr->m_opcode = CEE_RET;
    //rewriter.InsertAfter(pPrevInstr, pNewInstr);
    //rewriteHelper.Nop();

    rewriteHelper.SetILPosition(pFirstInstr);
    
    ////rewriteHelper.LoadNull();
    ////rewriteHelper.CallMember(testMemberRef, false);
    //rewriteHelper.Ret();
    //rewriteHelper.NewObject(ctorMemberRef);
    //rewriteHelper.StLocal(0);
    //rewriteHelper.LoadLocal(0);
    //rewriteHelper.CallMember(startMemberRef, true);

    // setup try/catch
    rewriteHelper.LoadNull();
    rewriteHelper.StLocal(indexMethodTrace);
    rewriteHelper.LoadNull();
    rewriteHelper.StLocal(indexEx);
    rewriteHelper.LoadNull();
    rewriteHelper.StLocal(indexRet);

    ILInstr* pTryStartInstr = rewriteHelper.CallMember0(getInstanceMemberRef, false);
    rewriteHelper.Cast(helperTypeRef);
    rewriteHelper.LoadToken(functionInfo.type.id);
    rewriteHelper.CallMember(getTypeFromHandleToken, false);
    rewriteHelper.LoadArgument(0);

    // call Start
    auto argNum = functionInfo.signature.NumberOfArguments();
    rewriteHelper.CreateArray(objectTypeRef, argNum);
    auto arguments = functionInfo.signature.GetMethodArguments();
    for (unsigned i = 0; i < argNum; i++) {
        rewriteHelper.BeginLoadValueIntoArray(i);
        rewriteHelper.LoadArgument(i + 1);
        auto argTypeFlags = arguments[i].GetTypeFlags(elementType);
        if (argTypeFlags & TypeFlagByRef) {
            rewriteHelper.LoadIND(elementType);
        }
        if (argTypeFlags & TypeFlagBoxedType) {
            auto tok = arguments[i].GetTypeTok(metadataEmit, mscorlibRef);
            if (tok == mdTokenNil) {
                return S_OK;
            }
            rewriteHelper.Box(tok);
        }
        rewriteHelper.EndLoadValueIntoArray();
    }

    rewriteHelper.LoadInt32((INT32)functionToken);
    rewriteHelper.CallMember(beforeMethodMemberRef, true);
    //rewriteHelper.Cast(startMemberRef);
    rewriteHelper.StLocal(rewriter.cNewLocals - 1);

    ILInstr* pRetInstr = rewriter.NewILInstr();
    pRetInstr->m_opcode = CEE_RET;
    rewriter.InsertAfter(rewriter.GetILList()->m_pPrev, pRetInstr);

    bool isVoidMethod = (retTypeFlags & TypeFlagVoid) > 0;
    auto ret = functionInfo.signature.GetRet();
    bool retIsBoxedType = false;
    mdToken retTypeTok;
    if (!isVoidMethod) {
        retTypeTok = ret.GetTypeTok(metadataEmit, mscorlibRef);
        if (ret.GetTypeFlags(elementType) & TypeFlagBoxedType) {
            retIsBoxedType = true;
        }
    }
    rewriteHelper.SetILPosition(pRetInstr);
    rewriteHelper.StLocal(indexEx);
    ILInstr* pRethrowInstr = rewriteHelper.Rethrow();

    rewriteHelper.LoadLocal(indexMethodTrace);
    ILInstr* pNewInstr = rewriter.NewILInstr();
    pNewInstr->m_opcode = CEE_BRFALSE_S;
    rewriter.InsertBefore(pRetInstr, pNewInstr);

    rewriteHelper.LoadLocal(indexMethodTrace);
    rewriteHelper.LoadLocal(indexRet);
    rewriteHelper.LoadLocal(indexEx);
    rewriteHelper.CallMember(finishMemberRef, true);

    ILInstr* pEndFinallyInstr = rewriteHelper.EndFinally();
    pNewInstr->m_pTarget = pEndFinallyInstr;

    if (!isVoidMethod) {
        rewriteHelper.LoadLocal(indexRet);
        if (retIsBoxedType) {
            rewriteHelper.UnboxAny(retTypeTok);
        }
        else {
            rewriteHelper.Cast(retTypeTok);
        }
    }

    for (ILInstr* pInstr = rewriter.GetILList()->m_pNext;
        pInstr != rewriter.GetILList();
        pInstr = pInstr->m_pNext) {
        switch (pInstr->m_opcode)
        {
        case CEE_RET:
        {
            if (pInstr != pRetInstr) {
                if (!isVoidMethod) {
                    rewriteHelper.SetILPosition(pInstr);
                    if (retIsBoxedType) {
                        rewriteHelper.Box(retTypeTok);
                    }
                    rewriteHelper.StLocal(indexRet);
                }
                pInstr->m_opcode = CEE_LEAVE_S;
                pInstr->m_pTarget = pEndFinallyInstr->m_pNext;
            }
            break;
        }
        default:
            break;
        }
    }

    // catch
    EHClause exClause{};
    exClause.m_Flags = COR_ILEXCEPTION_CLAUSE_NONE;
    exClause.m_pTryBegin = pTryStartInstr;
    exClause.m_pTryEnd = pRethrowInstr->m_pPrev;
    exClause.m_pHandlerBegin = pRethrowInstr->m_pPrev;
    exClause.m_pHandlerEnd = pRethrowInstr;
    exClause.m_ClassToken = exTypeRef;

    // finally
    EHClause finallyClause{};
    finallyClause.m_Flags = COR_ILEXCEPTION_CLAUSE_FINALLY;
    finallyClause.m_pTryBegin = pTryStartInstr;
    finallyClause.m_pTryEnd = pRethrowInstr->m_pNext;
    finallyClause.m_pHandlerBegin = pRethrowInstr->m_pNext;
    finallyClause.m_pHandlerEnd = pEndFinallyInstr;

    auto m_pEHNew = new EHClause[rewriter.m_nEH + 2];
    for (unsigned i = 0; i < rewriter.m_nEH; i++) {
        m_pEHNew[i] = rewriter.m_pEH[i];
    }

    rewriter.m_nEH += 2;
    m_pEHNew[rewriter.m_nEH - 2] = exClause;
    m_pEHNew[rewriter.m_nEH - 1] = finallyClause;
    rewriter.m_pEH = m_pEHNew;

    hr = rewriter.Export();
    IfFailRet(hr);

    return  S_OK;

    //return RewriteIL(this->corProfilerInfo, nullptr, moduleId, functionToken, functionId, reinterpret_cast<ULONGLONG>(EnterMethodAddress), reinterpret_cast<ULONGLONG>(LeaveMethodAddress), enterLeaveMethodSignatureToken);
}

HRESULT STDMETHODCALLTYPE CorProfiler::JITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::JITCachedFunctionSearchStarted(FunctionID functionId, BOOL* pbUseCachedFunction)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::JITCachedFunctionSearchFinished(FunctionID functionId, COR_PRF_JIT_CACHE result)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::JITFunctionPitched(FunctionID functionId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::JITInlining(FunctionID callerId, FunctionID calleeId, BOOL* pfShouldInline)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ThreadCreated(ThreadID threadId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ThreadDestroyed(ThreadID threadId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingClientInvocationStarted()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingClientSendingMessage(GUID* pCookie, BOOL fIsAsync)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingClientReceivingReply(GUID* pCookie, BOOL fIsAsync)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingClientInvocationFinished()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingServerInvocationStarted()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingServerInvocationReturned()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingServerSendingReply(GUID* pCookie, BOOL fIsAsync)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::UnmanagedToManagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ManagedToUnmanagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RuntimeSuspendFinished()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RuntimeSuspendAborted()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RuntimeResumeStarted()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RuntimeResumeFinished()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RuntimeThreadSuspended(ThreadID threadId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RuntimeThreadResumed(ThreadID threadId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::MovedReferences(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ObjectAllocated(ObjectID objectId, ClassID classId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ObjectsAllocatedByClass(ULONG cClassCount, ClassID classIds[], ULONG cObjects[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ObjectReferences(ObjectID objectId, ClassID classId, ULONG cObjectRefs, ObjectID objectRefIds[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RootReferences(ULONG cRootRefs, ObjectID rootRefIds[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionThrown(ObjectID thrownObjectId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionSearchFunctionEnter(FunctionID functionId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionSearchFunctionLeave()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionSearchFilterEnter(FunctionID functionId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionSearchFilterLeave()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionSearchCatcherFound(FunctionID functionId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionOSHandlerEnter(UINT_PTR __unused)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionOSHandlerLeave(UINT_PTR __unused)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionUnwindFunctionEnter(FunctionID functionId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionUnwindFunctionLeave()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionUnwindFinallyEnter(FunctionID functionId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionUnwindFinallyLeave()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionCatcherEnter(FunctionID functionId, ObjectID objectId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionCatcherLeave()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::COMClassicVTableCreated(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable, ULONG cSlots)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::COMClassicVTableDestroyed(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionCLRCatcherFound()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionCLRCatcherExecute()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ThreadNameChanged(ThreadID threadId, ULONG cchName, WCHAR name[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::GarbageCollectionStarted(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::GarbageCollectionFinished()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RootReferences2(ULONG cRootRefs, ObjectID rootRefIds[], COR_PRF_GC_ROOT_KIND rootKinds[], COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIds[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::HandleCreated(GCHandleID handleId, ObjectID initialObjectId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::HandleDestroyed(GCHandleID handleId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::InitializeForAttach(IUnknown* pCorProfilerInfoUnk, void* pvClientData, UINT cbClientData)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ProfilerAttachComplete()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ProfilerDetachSucceeded()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ReJITCompilationStarted(FunctionID functionId, ReJITID rejitId, BOOL fIsSafeToBlock)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::GetReJITParameters(ModuleID moduleId, mdMethodDef methodId, ICorProfilerFunctionControl* pFunctionControl)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ReJITCompilationFinished(FunctionID functionId, ReJITID rejitId, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ReJITError(ModuleID moduleId, mdMethodDef methodId, FunctionID functionId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::MovedReferences2(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], SIZE_T cObjectIDRangeLength[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::SurvivingReferences2(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], SIZE_T cObjectIDRangeLength[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ConditionalWeakTableElementReferences(ULONG cRootRefs, ObjectID keyRefIds[], ObjectID valueRefIds[], GCHandleID rootIds[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::GetAssemblyReferences(const WCHAR* wszAssemblyPath, ICorProfilerAssemblyReferenceProvider* pAsmRefProvider)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ModuleInMemorySymbolsUpdated(ModuleID moduleId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::DynamicMethodJITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock, LPCBYTE ilHeader, ULONG cbILHeader)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::DynamicMethodJITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
    return S_OK;
}
