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
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus)
{
    std::cout << "ModuleLoadFinished" << std::endl;
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
    HRESULT hres;
    mdToken functionToken;
    ClassID classId;
    ModuleID moduleId;

    IfFailRet(this->corProfilerInfo->GetFunctionInfo(functionId, &classId, &moduleId, &functionToken));

    ModuleInfo moduleInfo = GetModuleInfo(this->corProfilerInfo, moduleId);
    if (std::find(moduleNames.begin(), moduleNames.end(), moduleInfo.assembly.name) == moduleNames.end()) {
        return S_OK;
    }

    //std::cout << "JITCompilationStarted " << functionId << std::endl;

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    const auto pMetadataEmit =
        metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);

    mdSignature enterLeaveMethodSignatureToken;
    pMetadataEmit->GetTokenFromSig(enterLeaveMethodSignature, sizeof(enterLeaveMethodSignature), &enterLeaveMethodSignatureToken);

    const auto pMetadataAssemblyEmit =
        metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);

    const auto pMetadataImport =
        metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    auto functionInfo = GetFunctionInfo(pMetadataImport, functionToken);

    hres = functionInfo.signature.TryParse();
    IfFailRet(hres);

    // it's a hack, find a better way to figure out the entrypoint
    // i.e. catch the first call in an AppDomain
    if (ends_with(functionInfo.name, "Main"_W))
    {
        std::cout << "ENTRYPOINT!!" << std::endl;

        return LoadAssemblyBefore(this->corProfilerInfo,
            pMetadataImport,
            pMetadataEmit,
            pMetadataAssemblyEmit,
            nullptr,
            moduleId,
            functionToken,
            functionId,
            wrapperDllPath,
            enterLeaveMethodSignatureToken);
    }

    return Rewrite(moduleId, functionToken);
}

HRESULT CorProfiler::Rewrite(const ModuleID& moduleId, const mdToken& callerToken)
{
    HRESULT hres = 0;

    ILRewriter rewriter(this->corProfilerInfo, nullptr, moduleId, callerToken);
    IfFailRet(rewriter.Import());

    ILRewriterHelper reWriterWrapper(&rewriter);

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    const auto pMetadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);

    const auto pMetadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    const auto pMetadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);

    // for each IL instruction
    for (ILInstr* pInstr = rewriter.GetILList()->m_pNext;
        pInstr != rewriter.GetILList(); pInstr = pInstr->m_pNext)
    {
        // only CALL or CALLVIRT
        if (pInstr->m_opcode != CEE_CALL && pInstr->m_opcode != CEE_CALLVIRT) {
            continue;
        }

        auto functionToken = pInstr->m_Arg32;
        auto functionInfo = GetFunctionInfo(pMetadataImport, functionToken);
        hres = functionInfo.signature.TryParse();
        if ((functionInfo.name == "Test"_W
            || functionInfo.name == "ATest"_W
            || functionInfo.name == "Test1Async"_W
            || functionInfo.name == "Test2Async"_W
            || functionInfo.name == "TestVoid"_W
            ) &&
            (functionInfo.type.name == "SampleApp.Program"_W || functionInfo.type.name == "SampleApp.TestC"_W)) {
            std::cout << "Found call to " << ToString(functionInfo.type.name) << "." << ToString(functionInfo.name)
    << " num args " << functionInfo.signature.NumberOfArguments()
    << std::endl << std::flush;

            reWriterWrapper.SetILPosition(pInstr);

            // define mscorlib.dll
            mdModuleRef mscorlibRef;
            GetMsCorLibRef(hres, pMetadataAssemblyEmit, mscorlibRef);

            // define System.Object
            mdTypeRef objectTypeRef;
            hres = pMetadataEmit->DefineTypeRefByName(
                mscorlibRef,
                "System.Object"_W.data(),
                &objectTypeRef);
            IfFailRet(hres);

            // define wrapper.dll
            mdModuleRef wrapperRef;
            GetWrapperRef(hres, pMetadataAssemblyEmit, wrapperRef, wrapperAssemblyName);
            IfFailRet(hres);

            // define type System.Console
            mdTypeRef wrapperTypeRef;
            hres = pMetadataEmit->DefineTypeRefByName(
                wrapperRef,
                wrapperType.data(),
                &wrapperTypeRef);
            IfFailRet(hres);

            // rewrite
            unsigned elementType;
            auto retTypeFlags = functionInfo.signature.GetRet().GetTypeFlags(elementType);

            /*if (elementType != ELEMENT_TYPE_VOID)
            {
                std::cout << "NOT VOID" << std::endl;
                continue;
            }*/

            //std::cout << elementType << std::endl;

            auto argNum = functionInfo.signature.NumberOfArguments();

            /*if (argNum != 0 || functionInfo.signature.IsInstanceMethod())
            {
                continue;
            }*/

            unsigned inc = 0;

            if (functionInfo.signature.IsInstanceMethod())
            {
                inc++;
                argNum++;
            }

            reWriterWrapper.CreateArray(objectTypeRef, argNum);
            auto arguments = functionInfo.signature.GetMethodArguments();
            for (unsigned i = 0; i < argNum; i++) {
                std::cout << "load " << i << std::endl;
                reWriterWrapper.BeginLoadValueIntoArray(i);
                reWriterWrapper.LoadArgument(i);
                if (inc == 0 || i != 0)
                {
                    auto argTypeFlags = arguments[i - inc].GetTypeFlags(elementType);
                    if (argTypeFlags & TypeFlagByRef) {
                        reWriterWrapper.LoadIND(elementType);
                    }

                    if (argTypeFlags & TypeFlagBoxedType) {
                        auto tok = arguments[i - inc].GetTypeTok(pMetadataEmit, mscorlibRef);
                        if (tok == mdTokenNil) {
                            return S_OK;
                        }

                        reWriterWrapper.Box(tok);
                    }
                }

                reWriterWrapper.EndLoadValueIntoArray();
            }

            ILInstr* pNewInstr;

            {
                mdString typeNameToken;
                auto typeName = functionInfo.type.name;
                hres = pMetadataEmit->DefineUserString(typeName.data(), (ULONG)typeName.length(), &typeNameToken);

                pNewInstr = rewriter.NewILInstr();
                pNewInstr->m_opcode = CEE_LDSTR;
                pNewInstr->m_Arg32 = typeNameToken;
                rewriter.InsertBefore(pInstr, pNewInstr);
            }

            {
                mdString functionNameToken;
                auto functionName = functionInfo.name;
                hres = pMetadataEmit->DefineUserString(functionName.data(), (ULONG)functionName.length(), &functionNameToken);

                pNewInstr = rewriter.NewILInstr();
                pNewInstr->m_opcode = CEE_LDSTR;
                pNewInstr->m_Arg32 = functionNameToken;
                rewriter.InsertBefore(pInstr, pNewInstr);
            }

            reWriterWrapper.LoadInt32(functionToken);

            // method
            mdMethodDef testRef;

            if (argNum == 0)
            {
                BYTE Sig_void_String[] = {
                    IMAGE_CEE_CS_CALLCONV_DEFAULT,
                    4, // argument count
                    ELEMENT_TYPE_VOID,
                    ELEMENT_TYPE_SZARRAY,
                    ELEMENT_TYPE_OBJECT,
                    ELEMENT_TYPE_STRING,
                    ELEMENT_TYPE_STRING,
                    ELEMENT_TYPE_I4,
                };

                hres = pMetadataEmit->DefineMemberRef(
                    wrapperTypeRef,
                    "TestVoid"_W.data(),
                    Sig_void_String, sizeof(Sig_void_String),
                    &testRef);
                IfFailRet(hres);
            }
            else
            {
                BYTE Sig_void_String[] = {
                    IMAGE_CEE_CS_CALLCONV_DEFAULT,
                    4, // argument count
                    ELEMENT_TYPE_OBJECT,
                    ELEMENT_TYPE_SZARRAY,
                    ELEMENT_TYPE_OBJECT,
                    ELEMENT_TYPE_STRING,
                    ELEMENT_TYPE_STRING,
                    ELEMENT_TYPE_I4,
                };

                hres = pMetadataEmit->DefineMemberRef(
                    wrapperTypeRef,
                    "TestRet"_W.data(),
                    Sig_void_String, sizeof(Sig_void_String),
                    &testRef);
                IfFailRet(hres);
            }

            

            pNewInstr = rewriter.NewILInstr();
            pNewInstr->m_opcode = CEE_CALL;
            pNewInstr->m_Arg32 = testRef;
            rewriter.InsertBefore(pInstr, pNewInstr);

            pInstr->m_opcode = CEE_NOP;
        }
    }

    IfFailRet(rewriter.Export(false));

    return S_OK;
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
