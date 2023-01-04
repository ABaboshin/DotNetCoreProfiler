#include <algorithm>
#include <iostream>
#include <string>
#include "corhlpr.h"
#include "corhdr.h"
#include "configuration/Configuration.h"
#include "const/const.h"
#include "info/InterceptionVarInfo.h"
#include "info/parser.h"
#include "rewriter/ILRewriterHelper.h"
#include "util/helpers.h"
#include "util/util.h"
#include "util/ComPtr.h"
#include "CorProfiler.h"
#include "dllmain.h"
#include "logging/logging.h"
#include <functional>

CorProfiler* instance = nullptr;

CorProfiler::CorProfiler() : refCount(0), corProfilerInfo(nullptr), methodRewriter(this)
{
    logging::init();
#define OPDEF(c, s, pop, push, args, type, l, s1, s2, flow) opCodes.push_back(s);
#include "opcode.def"
#undef OPDEF
    opCodes.push_back("(count)");
    opCodes.push_back("->");
    instance = this;
    debuggerThread = std::make_unique<std::thread>(ProcessDebuggerRejits);
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

    eventMask = COR_PRF_MONITOR_JIT_COMPILATION |
                      COR_PRF_DISABLE_TRANSPARENCY_CHECKS_UNDER_FULL_TRUST |
                      // COR_PRF_DISABLE_INLINING |
                      COR_PRF_MONITOR_MODULE_LOADS |
                      COR_PRF_MONITOR_ASSEMBLY_LOADS | COR_PRF_MONITOR_APPDOMAIN_LOADS |
                      //COR_PRF_DISABLE_ALL_NGEN_IMAGES |
                      COR_PRF_ENABLE_REJIT;

    auto hr = this->corProfilerInfo->SetEventMask(eventMask);
    if (FAILED(hr)) {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed SetEventMask"_W);
        return hr;
    }

    configuration = configuration::Configuration::LoadConfiguration(GetEnvironmentValue("PROFILER_CONFIGURATION"));

    oneAppDomainMode = util::GetEnvironmentValue("PROFILER_ONE_APP_DOMAIN") == "1";

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


std::vector<configuration::InstrumentationConfiguration> CorProfiler::FindInterceptors(const info::TypeInfo& typeInfo, const info::FunctionInfo& functionInfo) {
    std::vector<configuration::InstrumentationConfiguration> result{};

    for (const auto& el : configuration.Instrumentations)
    {
        if (typeInfo.Name == el.first.TypeName
            && functionInfo.Name == el.first.MethodName && functionInfo.Signature.NumberOfArguments() == el.first.MethodParametersCount)
        {
            result.push_back(el.second);
        }
    }

    if (typeInfo.ParentTypeInfo != nullptr)
    {
        auto r2 = FindInterceptors(*typeInfo.ParentTypeInfo, functionInfo);
        std::copy(r2.begin(), r2.end(), std::back_inserter(result));
    }

    if (typeInfo.ExtendTypeInfo != nullptr)
    {
        auto r2 = FindInterceptors(*typeInfo.ExtendTypeInfo, functionInfo);
        std::copy(r2.begin(), r2.end(), std::back_inserter(result));
    }

    return result;
}

std::tuple<bool, std::vector<util::wstring>, util::wstring> CorProfiler::FindTraces(const info::TypeInfo& typeInfo, const info::FunctionInfo& functionInfo) {
    std::vector<configuration::TraceMethodInfo> result{};

    for (const auto& el : configuration.Instrumentations)
    {
        if (typeInfo.Name == el.first.TypeName
            && functionInfo.Name == el.first.MethodName && functionInfo.Signature.NumberOfArguments() == el.first.MethodParametersCount && el.second.Trace)
        {
            return std::tuple<bool, std::vector<util::wstring>, util::wstring>(true, el.second.Parameters, el.second.TraceName);
        }
    }

    if (typeInfo.ParentTypeInfo != nullptr)
    {
        auto x = FindTraces(*typeInfo.ParentTypeInfo, functionInfo);
        if (std::get<0>(x))
        {
            return x;
        }
    }

    if (typeInfo.ExtendTypeInfo != nullptr)
    {
        auto x = FindTraces(*typeInfo.ExtendTypeInfo, functionInfo);
        if (std::get<0>(x))
        {
            return x;
        }
    }

    return std::tuple<bool, std::vector<util::wstring>, util::wstring>(false, {}, ""_W);
}

std::vector<int> CorProfiler::FindOffsets(const info::TypeInfo& typeInfo, const info::FunctionInfo& functionInfo) {
    for (const auto& el : configuration.Instrumentations)
    {
        if (typeInfo.Name == el.first.TypeName
            && functionInfo.Name == el.first.MethodName && functionInfo.Signature.NumberOfArguments() == el.first.MethodParametersCount)
        {
            return el.second.Offsets;
        }
    }

    return {};
}

std::vector<info::TypeInfo> CorProfiler::GetAllImplementedInterfaces(const info::TypeInfo typeInfo, util::ComPtr<IMetaDataImport2>& metadataImport)
{
    // get all implemented interfaces
    HCORENUM hcorenumInterfaces = 0;
    mdTypeDef interfaces[MAX_CLASS_NAME]{};
    ULONG interfacesCount;
    auto hr = metadataImport->EnumInterfaceImpls(&hcorenumInterfaces, typeInfo.Id, interfaces, MAX_CLASS_NAME, &interfacesCount);
    if (FAILED(hr)) {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed ModuleLoadFinished EnumInterfaceImpls"_W);
        return {};
    }

    std::vector<info::TypeInfo> interfaceInfos;
    for (auto interfaceIndex = 0; interfaceIndex < interfacesCount; interfaceIndex++)
    {
        mdToken classToken, interfaceToken;
        hr = metadataImport->GetInterfaceImplProps(interfaces[interfaceIndex], &classToken, &interfaceToken);
        if (hr == S_OK && classToken == typeInfo.Id)
        {
            auto interfaceInfo = info::TypeInfo::GetTypeInfo(metadataImport, interfaceToken);
            interfaceInfos.push_back(interfaceInfo);
        }
    }

    if (typeInfo.ExtendTypeInfo != nullptr)
    {
        auto extend = GetAllImplementedInterfaces(*typeInfo.ExtendTypeInfo, metadataImport);
        std::copy(extend.begin(), extend.end(), std::back_inserter(interfaceInfos));
    }

    if (typeInfo.ParentTypeInfo != nullptr)
    {
        auto extend = GetAllImplementedInterfaces(*typeInfo.ParentTypeInfo, metadataImport);
        std::copy(extend.begin(), extend.end(), std::back_inserter(interfaceInfos));
    }

    return interfaceInfos;
}

void CorProfiler::AddMethodParameters(util::wstring assemblyName, util::wstring typeName, util::wstring methodName, int methodParametersCount, std::vector<util::wstring> parameters)
{
    std::lock_guard<std::mutex> guard(mutex);

    logging::log(logging::LogLevel::INFO, "AddMethodParameters {0}.{1}.{2} {3} {4}"_W, assemblyName, typeName, methodName, methodParametersCount, parameters.size());

    configuration::TargetMethod tm(assemblyName, typeName, methodName, methodParametersCount);
    auto it = configuration.Instrumentations.find(tm);
    if (it == configuration.Instrumentations.end())
    {
        std::pair<configuration::TargetMethod, configuration::InstrumentationConfiguration> ti(tm, {});
        configuration.Instrumentations.insert(ti);

        it = configuration.Instrumentations.find(tm);
    }

    it->second.Parameters = parameters;
}

void CorProfiler::AddMethodVariables(util::wstring assemblyName, util::wstring typeName, util::wstring methodName, int methodParametersCount, std::vector<util::wstring> variables)
{
    std::lock_guard<std::mutex> guard(mutex);

    logging::log(logging::LogLevel::INFO, "AddMethodVariables {0}.{1}.{2} {3} {4}"_W, assemblyName, typeName, methodName, methodParametersCount, variables.size());

    configuration::TargetMethod tm(assemblyName, typeName, methodName, methodParametersCount);
    auto it = configuration.Instrumentations.find(tm);
    if (it == configuration.Instrumentations.end())
    {
        std::pair<configuration::TargetMethod, configuration::InstrumentationConfiguration> ti(tm, {});
        configuration.Instrumentations.insert(ti);

        it = configuration.Instrumentations.find(tm);
    }

    it->second.Variables = variables;
}

void CorProfiler::StartDebugger()
{
    logging::log(logging::LogLevel::INFO, "StartDebugger"_W);
    debuggerQueue.push(true);
}

void CorProfiler::AddDebuggerOffset(util::wstring assemblyName, util::wstring typeName, util::wstring methodName, int methodParametersCount, int offset)
{
    std::lock_guard<std::mutex> guard(mutex);

    logging::log(logging::LogLevel::INFO, "AddDebuggerOffset {0} {1} {2} {3} {4}"_W, assemblyName, typeName, methodName, methodParametersCount, offset);

    configuration::TargetMethod tm(assemblyName, typeName, methodName, methodParametersCount);
    auto it = configuration.Instrumentations.find(tm);
    if (it == configuration.Instrumentations.end())
    {
        std::pair<configuration::TargetMethod, configuration::InstrumentationConfiguration> ti(tm, {});
        configuration.Instrumentations.insert(ti);

        it = configuration.Instrumentations.find(tm);
    }

    it->second.Offsets.push_back(offset);
}

void CorProfiler::ProcessDebuggerRejits()
{
    HRESULT hr = instance->corProfilerInfo->InitializeCurrentThread();
    if (FAILED(true))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "ProcessDebuggerRejits"_W);
    }

    while (true)
    {
        bool b = false;
        instance->debuggerQueue.waitAndPop(b);
        logging::log(logging::LogLevel::INFO, "StartDebugger received"_W);

        for (const auto& instr : instance->configuration.Instrumentations)
        {
            if (instr.second.Offsets.empty())
            {
                continue;
            }

            for (const auto& el : instance->loadedModules)
            {
                if (el.first == instr.first.AssemblyName)
                {
                    std::cout << 1 << std::endl;

                    ComPtr<IUnknown> metadataInterfaces;
                    std::cout << 11 << std::endl;
                    // 0x80131363 = CORPROF_E_UNSUPPORTED_CALL_SEQUENCE
                    auto hr = instance->corProfilerInfo->GetModuleMetaData(el.second, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf());
                    std::cout << 12 << std::endl;
                    if (FAILED(hr))
                    {
                        std::cout << 13 << std::endl;
                        std::cout << std::hex << hr << std::endl;
                        logging::log(logging::LogLevel::NONSUCCESS, "Failed ModuleLoadFinished GetModuleMetaData"_W);
                        return;
                    }

                    std::cout << 2 << std::endl;

                    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

                    std::cout << 3 << std::endl;

                    HCORENUM hcorenumTypeDefs = 0;
                    mdTypeDef typeDefs[MAX_CLASS_NAME]{};
                    ULONG typeDefsCount;
                    hr = metadataImport->EnumTypeDefs(&hcorenumTypeDefs, typeDefs, MAX_CLASS_NAME, &typeDefsCount);
                    if (FAILED(hr)) {
                        logging::log(logging::LogLevel::NONSUCCESS, "Failed ModuleLoadFinished EnumTypeDefs"_W);
                        return;
                    }

                    std::cout << 4 << std::endl;

                    for (auto typeIndex = 0; typeIndex < typeDefsCount; typeIndex++) {
                        const auto typeInfo = info::TypeInfo::GetTypeInfo(metadataImport, typeDefs[typeIndex]);

                        logging::log(logging::LogLevel::INFO, "x {0} {1}"_W, typeInfo.Name, instr.first.TypeName);

                        if (typeInfo.Name != instr.first.TypeName)
                        {
                            continue;
                        }

                        std::cout << 5 << std::endl;

                        // get all methods
                        HCORENUM hcorenumMethods = 0;
                        mdMethodDef methods[MAX_CLASS_NAME]{};
                        ULONG methodsCount;
                        hr = metadataImport->EnumMethods(&hcorenumMethods, typeDefs[typeIndex], methods, MAX_CLASS_NAME, &methodsCount);
                        if (FAILED(hr)) {
                            logging::log(logging::LogLevel::NONSUCCESS, "Failed ModuleLoadFinished EnumInterfaceImpls"_W);
                            return;
                        }

                        for (auto methodIndex = 0; methodIndex < methodsCount; methodIndex++) {
                            auto functionInfo = info::FunctionInfo::GetFunctionInfo(metadataImport, methods[methodIndex]);

                            if (functionInfo.Name != instr.first.MethodName || functionInfo.Signature.NumberOfArguments() != instr.first.MethodParametersCount)
                            {
                                continue;
                            }

                            ModuleID m1[1]{ el.second };
                            mdMethodDef m2[1]{ methods[methodIndex] };

                            auto rit = std::find_if(instance->rejitInfo.begin(), instance->rejitInfo.end(), [el, methods, methodIndex](const RejitInfo& ri) {
                                return ri.ModuleId == el.second && methods[methodIndex] == ri.MethodId;
                            });

                            if (rit != instance->rejitInfo.end())
                            {
                                rit->Offsets = instr.second.Offsets;
                            }
                            else
                            {
                                auto ri = RejitInfo(el.second, methods[methodIndex], functionInfo, {}, {}, {}, {}, instr.second.Offsets);
                                instance->rejitInfo.push_back(ri);
                            }

                            // and then request rejit
                            hr = instance->corProfilerInfo->RequestReJIT(1, m1, m2);

                            logging::log(logging::LogLevel::INFO, "Rejit {0}.{1} for debug"_W, instr.first.TypeName, instr.first.MethodName);
                            return;
                        }
                    }
                }
            }
        }
    }
}