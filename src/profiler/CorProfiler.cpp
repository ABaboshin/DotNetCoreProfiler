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
#include "clr_const.h"

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
        COR_PRF_DISABLE_TRANSPARENCY_CHECKS_UNDER_FULL_TRUST | 
        COR_PRF_DISABLE_INLINING | COR_PRF_MONITOR_MODULE_LOADS |
        COR_PRF_DISABLE_ALL_NGEN_IMAGES;

    auto hr = this->corProfilerInfo->SetEventMask(eventMask);

    is_attached = true;

    interceptions = LoadFromFile(GetEnvironmentValue("PROFILER_CONFIGURATION"_W));

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
    HRESULT hr;
    const auto module_info = GetModuleInfo(this->corProfilerInfo, moduleId);
    auto app_domain_id = module_info.assembly.app_domain_id;

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    const auto pMetadataImport =
        metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    mdModule module;
    hr = pMetadataImport->GetModuleFromScope(&module);

    GUID module_version_id;
    hr = pMetadataImport->GetScopeProps(nullptr, 0, nullptr, &module_version_id);

    modules[moduleId] = module_version_id;

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

    auto moduleInfo = GetModuleInfo(this->corProfilerInfo, moduleId);

    // if the current call is not a call to one of skipped assemblies
    if (SkipAssembly(moduleInfo.assembly.name))
    {
        return S_OK;
    }

    // load once into appdomain
    if (loadedIntoAppDomains.find(moduleInfo.assembly.app_domain_id) == loadedIntoAppDomains.end())
    {
        loadedIntoAppDomains.insert(moduleInfo.assembly.app_domain_id);

        ComPtr<IUnknown> metadataInterfaces;
        IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

        const auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

        auto functionInfo = GetFunctionInfo(metadataImport, functionToken);

        hr = functionInfo.signature.TryParse();
        IfFailRet(hr);

        std::cout << "Load into app_domain_id " << moduleInfo.assembly.app_domain_id 

        << "Before call to " << ToString(functionInfo.type.name) << "." << ToString(functionInfo.name)
            << " num args " << functionInfo.signature.NumberOfArguments()
            << " from assembly " << ToString(moduleInfo.assembly.name)
            << std::endl << std::flush;

        std::vector<WSTRING> dlls;
        std::for_each(interceptions.begin(), interceptions.end(), [&dlls](Interception i) { dlls.push_back(i.WrapperAssemblyPath); });

        std::vector<WSTRING> udlls;
        std::copy_if(dlls.begin(), dlls.end(), std::back_inserter(udlls), [&udlls](WSTRING p) {
            return std::find(udlls.begin(), udlls.end(), p) == udlls.end();
        });

        return LoadAssemblyBefore(
            nullptr,
            moduleId,
            functionToken,
            functionId,
            udlls);
    }

    return Rewrite(moduleId, functionToken);
}

HRESULT CorProfiler::LoadAssemblyBefore(
    ICorProfilerFunctionControl* pICorProfilerFunctionControl,
    ModuleID moduleId,
    mdMethodDef methodDef,
    FunctionID functionId,
    std::vector<WSTRING> assemblies)
{
    HRESULT hr;

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    const auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);

    const auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);

    const auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    ILRewriter rewriter(this->corProfilerInfo, pICorProfilerFunctionControl, moduleId, methodDef);

    IfFailRet(rewriter.Import());

    ILInstr* pFirstInstr = rewriter.GetILList()->m_pNext;

    for (const auto& assemblyPath : assemblies)
    {
        mdString aPath;
        hr = metadataEmit->DefineUserString(assemblyPath.c_str(), (ULONG)assemblyPath.length(),
            &aPath);

        ULONG string_len = 0;
        WCHAR string_contents[NameMaxSize]{};
        hr = metadataImport->GetUserString(aPath, string_contents,
            NameMaxSize, &string_len);
        IfFailRet(hr);


        // define mscorlib.dll
        mdModuleRef mscorlibRef;
        GetMsCorLibRef(hr, metadataAssemblyEmit, mscorlibRef);
        IfFailRet(hr);

        // define type System.Reflection.Assembly
        mdTypeRef assemblyTypeRef;
        hr = metadataEmit->DefineTypeRefByName(
            mscorlibRef,
            "System.Reflection.Assembly"_W.data(),
            &assemblyTypeRef);

        unsigned buffer;
        auto size = CorSigCompressToken(assemblyTypeRef, &buffer);
        auto* assemblyLoadSig = new COR_SIGNATURE[size + 4];
        unsigned offset = 0;
        assemblyLoadSig[offset++] = IMAGE_CEE_CS_CALLCONV_DEFAULT;
        assemblyLoadSig[offset++] = 0x01;
        assemblyLoadSig[offset++] = ELEMENT_TYPE_CLASS;
        memcpy(&assemblyLoadSig[offset], &buffer, size);
        offset += size;
        assemblyLoadSig[offset] = ELEMENT_TYPE_STRING;

        // define method System.Reflection.Assembly.LoadFrom
        mdMemberRef assemblyLoadMemberRef;
        hr = metadataEmit->DefineMemberRef(
            assemblyTypeRef,
            "LoadFrom"_W.data(),
            assemblyLoadSig,
            sizeof(assemblyLoadSig),
            &assemblyLoadMemberRef);

        // define path to a .net dll 
        mdString profilerTraceDllNameTextToken;
        hr = metadataEmit->DefineUserString(assemblyPath.data(), (ULONG)assemblyPath.length(), &profilerTraceDllNameTextToken);

        std::cout << "LoadAssemblyBefore " << ToString(assemblyPath) << std::endl;

        // load path
        ILInstr* pNewInstr = rewriter.NewILInstr();
        pNewInstr->m_opcode = CEE_LDSTR;
        pNewInstr->m_Arg32 = profilerTraceDllNameTextToken;
        rewriter.InsertBefore(pFirstInstr, pNewInstr);

        // call System.Reflection.Assembly.LoadFrom
        pNewInstr = rewriter.NewILInstr();
        pNewInstr->m_opcode = CEE_CALL;
        pNewInstr->m_Arg32 = assemblyLoadMemberRef;
        rewriter.InsertBefore(pFirstInstr, pNewInstr);

        // clean stack
        pNewInstr = rewriter.NewILInstr();
        pNewInstr->m_opcode = CEE_POP;
        rewriter.InsertBefore(pFirstInstr, pNewInstr);
    }

    IfFailRet(rewriter.Export(false));

    return S_OK;
}

bool CorProfiler::SkipAssembly(const WSTRING& name)
{
    std::vector<WSTRING> skipAssemblies{
      "mscorlib"_W,
      "netstandard"_W,
      "System.Core"_W,
      "System.Runtime"_W,
      "System.IO.FileSystem"_W,
      "System.Collections"_W,
      "System.Runtime.Extensions"_W,
      "System.Threading.Tasks"_W,
      "System.Runtime.InteropServices"_W,
      "System.Runtime.InteropServices.RuntimeInformation"_W,
      "System.ComponentModel"_W,
      "System.Console"_W,
      "System.Diagnostics.DiagnosticSource"_W,
      "System.Private.CoreLib"_W,
      "Microsoft.Extensions.Options"_W,
      "Microsoft.Extensions.ObjectPool"_W,
      "System.Configuration"_W,
      "System.Xml.Linq"_W,
      "Microsoft.AspNetCore.Razor.Language"_W,
      "Microsoft.AspNetCore.Mvc.RazorPages"_W,
      "Microsoft.CSharp"_W,
      "Anonymously Hosted DynamicMethods Assembly"_W,
      "ISymWrapper"_W,
      "Interception"_W,
      "Interception.Common"_W,
      "Interception.Executor"_W,
      "Interception.Generator"_W,
      "Interception.Metrics"_W,
      "Interception.Observers"_W,
      "StatsdClient"_W,
      "Newtonsoft.Json"_W
    };

    return std::find(skipAssemblies.begin(), skipAssemblies.end(), name) != skipAssemblies.end();
}

HRESULT CorProfiler::Rewrite(const ModuleID& moduleId, const mdToken& callerToken)
{
    HRESULT hr;

    ILRewriter rewriter(this->corProfilerInfo, nullptr, moduleId, callerToken);
    IfFailRet(rewriter.Import());

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    const auto pMetadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);

    const auto pMetadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    const auto pMetadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);

    auto functionInfo = GetFunctionInfo(pMetadataImport, callerToken);
    hr = functionInfo.signature.TryParse();

    auto moduleInfo = GetModuleInfo(this->corProfilerInfo, moduleId);

    if (!SkipAssembly(moduleInfo.assembly.name)) for (ILInstr* pInstr = rewriter.GetILList()->m_pNext;
        pInstr != rewriter.GetILList(); pInstr = pInstr->m_pNext) {
        if (pInstr->m_opcode != CEE_CALL && pInstr->m_opcode != CEE_CALLVIRT) {
            continue;
        }

        auto target =
            GetFunctionInfo(pMetadataImport, pInstr->m_Arg32);
        target.signature.TryParse();

        auto targetMdToken = pInstr->m_Arg32;

        //std::cout << "Found call to " << ToString(target.type.name) << "." << ToString(target.name)
        //    << " num args " << target.signature.NumberOfArguments()
        //    << " from assembly " << ToString(moduleInfo.assembly.name)
        //    << std::endl << std::flush;

        for (const auto& interception : interceptions)
        {
            if (
                (moduleInfo.assembly.name == interception.CallerAssemblyName || interception.CallerAssemblyName.empty())
                && target.type.name == interception.TargetTypeName
                && target.name == interception.TargetMethodName && interception.TargetMethodParametersCount == target.signature.NumberOfArguments()
                )
            {
                auto m = modules[moduleId];

                std::cout << "Found call to " << ToString(target.type.name) << "." << ToString(target.name)
                    << " num args " << target.signature.NumberOfArguments()
                    << " from assembly " << ToString(moduleInfo.assembly.name)
                    << " module " << m
                    << std::endl << std::flush;
                auto signature = interception.signature;

                // define wrapper.dll
                mdModuleRef wrapperRef;
                GetWrapperRef(hr, pMetadataAssemblyEmit, wrapperRef, interception.WrapperAssemblyName);
                IfFailRet(hr);

                // define wrappedType
                mdTypeRef wrapperTypeRef;
                hr = pMetadataEmit->DefineTypeRefByName(
                    wrapperRef,
                    interception.WrapperTypeName.data(),
                    &wrapperTypeRef);
                IfFailRet(hr);

                // method
                mdMemberRef wrapperMethodRef;
                hr = pMetadataEmit->DefineMemberRef(
                    wrapperTypeRef, interception.WrapperMethodName.c_str(),
                    signature.data(),
                    (DWORD)(signature.size()),
                    &wrapperMethodRef);

                ILRewriterHelper helper(&rewriter);
                helper.SetILPosition(pInstr);
                helper.LoadInt32(targetMdToken);

                const void* module_version_id_ptr = &modules[moduleId];

                helper.LoadInt64(reinterpret_cast<INT64>(module_version_id_ptr));

                helper.CallMember(wrapperMethodRef, false);

                pInstr->m_opcode = CEE_NOP;
                            
                IfFailRet(rewriter.Export(false));
            }
        }
    }

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
