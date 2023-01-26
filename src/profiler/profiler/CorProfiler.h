#pragma once

#include <atomic>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <mutex>
#include <thread>
#include "cor.h"
#include "corprof.h"
#include "info/FunctionInfo.h"
#include "info/ModuleInfo.h"
#include "configuration/Configuration.h"
#include "rewriter/ILRewriter.h"
#include "RejitInfo.h"
#include "MethodRewriter.h"

#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class LockingQueue
{
public:
    void push(T const& _data)
    {
        {
            std::lock_guard<std::mutex> lock(guard);
            queue.push(_data);
        }
        signal.notify_one();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(guard);
        return queue.empty();
    }

    bool tryPop(T& _value)
    {
        std::lock_guard<std::mutex> lock(guard);
        if (queue.empty())
        {
            return false;
        }

        _value = queue.front();
        queue.pop();
        return true;
    }

    void waitAndPop(T& _value)
    {
        std::unique_lock<std::mutex> lock(guard);
        while (queue.empty())
        {
            signal.wait(lock);
        }

        _value = queue.front();
        queue.pop();
    }

    bool tryWaitAndPop(T& _value, int _milli)
    {
        std::unique_lock<std::mutex> lock(guard);
        while (queue.empty())
        {
            signal.wait_for(lock, std::chrono::milliseconds(_milli));
            return false;
        }

        _value = queue.front();
        queue.pop();
        return true;
    }

private:
    std::queue<T> queue;
    mutable std::mutex guard;
    std::condition_variable signal;
};

class CorProfiler : public ICorProfilerCallback8
{
protected:
    LockingQueue<bool> debuggerQueue;
    std::unique_ptr<std::thread> debuggerThread;
    std::vector<std::string> opCodes;
    DWORD eventMask;
    bool oneAppDomainMode = false;
    MethodRewriter methodRewriter;
    std::atomic<int> refCount;

    std::unordered_set<AppDomainID> loadedIntoAppDomains;

    std::unordered_set<ModuleID> skippedModules;
    std::vector<RejitInfo> rejitInfo;
    std::unordered_map<util::wstring, ModuleID> loadedModules;

    std::mutex mutex;
    bool alreadyLoaded;

    configuration::Configuration configuration{};
    ICorProfilerInfo8* corProfilerInfo;

    bool SkipAssembly(const wstring& name);

    virtual HRESULT InjectLoadMethod(ModuleID moduleId, rewriter::ILRewriter& rewriter, const info::FunctionInfo& functionInfo);

    HRESULT GenerateLoadMethod(ModuleID moduleId, mdMethodDef& retMethodToken, const info::FunctionInfo& functionInfo);

    virtual rewriter::ILRewriter* CreateILRewriter(ICorProfilerFunctionControl* pICorProfilerFunctionControl, ModuleID moduleId, mdToken functionToken)
    {
        return new rewriter::ILRewriter(this->corProfilerInfo, nullptr, moduleId, functionToken);
    }

    std::vector<configuration::InstrumentationConfiguration> FindInterceptors(const info::TypeInfo& typeInfo, const info::FunctionInfo& functionInfo);
    std::tuple<bool, std::vector<util::wstring>, util::wstring> FindTraces(const info::TypeInfo& typeInfo, const info::FunctionInfo& functionInfo);
    std::vector<int> FindOffsets(const info::TypeInfo& typeInfo, const info::FunctionInfo& functionInfo);

    std::vector<info::TypeInfo> GetAllImplementedInterfaces(const info::TypeInfo typeInfo, util::ComPtr<IMetaDataImport2>& metadataImport);

    util::wstring DumpILCodes(rewriter::ILRewriter* rewriter, const info::FunctionInfo& functionInfo, const ComPtr<IMetaDataImport2>& metadataImport);

    static void ProcessDebuggerRejits();

public:
    CorProfiler();
    virtual ~CorProfiler();
    HRESULT STDMETHODCALLTYPE Initialize(IUnknown* pICorProfilerInfoUnk) override;
    HRESULT STDMETHODCALLTYPE Shutdown() override;
    HRESULT STDMETHODCALLTYPE AppDomainCreationStarted(AppDomainID appDomainId) override;
    HRESULT STDMETHODCALLTYPE AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus) override;
    HRESULT STDMETHODCALLTYPE AppDomainShutdownStarted(AppDomainID appDomainId) override;
    HRESULT STDMETHODCALLTYPE AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus) override;
    HRESULT STDMETHODCALLTYPE AssemblyLoadStarted(AssemblyID assemblyId) override;
    HRESULT STDMETHODCALLTYPE AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus) override;
    HRESULT STDMETHODCALLTYPE AssemblyUnloadStarted(AssemblyID assemblyId) override;
    HRESULT STDMETHODCALLTYPE AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus) override;
    HRESULT STDMETHODCALLTYPE ModuleLoadStarted(ModuleID moduleId) override;
    HRESULT STDMETHODCALLTYPE ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus) override;
    HRESULT STDMETHODCALLTYPE ModuleUnloadStarted(ModuleID moduleId) override;
    HRESULT STDMETHODCALLTYPE ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus) override;
    HRESULT STDMETHODCALLTYPE ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID AssemblyId) override;
    HRESULT STDMETHODCALLTYPE ClassLoadStarted(ClassID classId) override;
    HRESULT STDMETHODCALLTYPE ClassLoadFinished(ClassID classId, HRESULT hrStatus) override;
    HRESULT STDMETHODCALLTYPE ClassUnloadStarted(ClassID classId) override;
    HRESULT STDMETHODCALLTYPE ClassUnloadFinished(ClassID classId, HRESULT hrStatus) override;
    HRESULT STDMETHODCALLTYPE FunctionUnloadStarted(FunctionID functionId) override;
    HRESULT STDMETHODCALLTYPE JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock) override;
    HRESULT STDMETHODCALLTYPE JITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock) override;
    HRESULT STDMETHODCALLTYPE JITCachedFunctionSearchStarted(FunctionID functionId, BOOL* pbUseCachedFunction) override;
    HRESULT STDMETHODCALLTYPE JITCachedFunctionSearchFinished(FunctionID functionId, COR_PRF_JIT_CACHE result) override;
    HRESULT STDMETHODCALLTYPE JITFunctionPitched(FunctionID functionId) override;
    HRESULT STDMETHODCALLTYPE JITInlining(FunctionID callerId, FunctionID calleeId, BOOL* pfShouldInline) override;
    HRESULT STDMETHODCALLTYPE ThreadCreated(ThreadID threadId) override;
    HRESULT STDMETHODCALLTYPE ThreadDestroyed(ThreadID threadId) override;
    HRESULT STDMETHODCALLTYPE ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId) override;
    HRESULT STDMETHODCALLTYPE RemotingClientInvocationStarted() override;
    HRESULT STDMETHODCALLTYPE RemotingClientSendingMessage(GUID* pCookie, BOOL fIsAsync) override;
    HRESULT STDMETHODCALLTYPE RemotingClientReceivingReply(GUID* pCookie, BOOL fIsAsync) override;
    HRESULT STDMETHODCALLTYPE RemotingClientInvocationFinished() override;
    HRESULT STDMETHODCALLTYPE RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync) override;
    HRESULT STDMETHODCALLTYPE RemotingServerInvocationStarted() override;
    HRESULT STDMETHODCALLTYPE RemotingServerInvocationReturned() override;
    HRESULT STDMETHODCALLTYPE RemotingServerSendingReply(GUID* pCookie, BOOL fIsAsync) override;
    HRESULT STDMETHODCALLTYPE UnmanagedToManagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason) override;
    HRESULT STDMETHODCALLTYPE ManagedToUnmanagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason) override;
    HRESULT STDMETHODCALLTYPE RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason) override;
    HRESULT STDMETHODCALLTYPE RuntimeSuspendFinished() override;
    HRESULT STDMETHODCALLTYPE RuntimeSuspendAborted() override;
    HRESULT STDMETHODCALLTYPE RuntimeResumeStarted() override;
    HRESULT STDMETHODCALLTYPE RuntimeResumeFinished() override;
    HRESULT STDMETHODCALLTYPE RuntimeThreadSuspended(ThreadID threadId) override;
    HRESULT STDMETHODCALLTYPE RuntimeThreadResumed(ThreadID threadId) override;
    HRESULT STDMETHODCALLTYPE MovedReferences(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[]) override;
    HRESULT STDMETHODCALLTYPE ObjectAllocated(ObjectID objectId, ClassID classId) override;
    HRESULT STDMETHODCALLTYPE ObjectsAllocatedByClass(ULONG cClassCount, ClassID classIds[], ULONG cObjects[]) override;
    HRESULT STDMETHODCALLTYPE ObjectReferences(ObjectID objectId, ClassID classId, ULONG cObjectRefs, ObjectID objectRefIds[]) override;
    HRESULT STDMETHODCALLTYPE RootReferences(ULONG cRootRefs, ObjectID rootRefIds[]) override;
    HRESULT STDMETHODCALLTYPE ExceptionThrown(ObjectID thrownObjectId) override;
    HRESULT STDMETHODCALLTYPE ExceptionSearchFunctionEnter(FunctionID functionId) override;
    HRESULT STDMETHODCALLTYPE ExceptionSearchFunctionLeave() override;
    HRESULT STDMETHODCALLTYPE ExceptionSearchFilterEnter(FunctionID functionId) override;
    HRESULT STDMETHODCALLTYPE ExceptionSearchFilterLeave() override;
    HRESULT STDMETHODCALLTYPE ExceptionSearchCatcherFound(FunctionID functionId) override;
    HRESULT STDMETHODCALLTYPE ExceptionOSHandlerEnter(UINT_PTR __unused) override;
    HRESULT STDMETHODCALLTYPE ExceptionOSHandlerLeave(UINT_PTR __unused) override;
    HRESULT STDMETHODCALLTYPE ExceptionUnwindFunctionEnter(FunctionID functionId) override;
    HRESULT STDMETHODCALLTYPE ExceptionUnwindFunctionLeave() override;
    HRESULT STDMETHODCALLTYPE ExceptionUnwindFinallyEnter(FunctionID functionId) override;
    HRESULT STDMETHODCALLTYPE ExceptionUnwindFinallyLeave() override;
    HRESULT STDMETHODCALLTYPE ExceptionCatcherEnter(FunctionID functionId, ObjectID objectId) override;
    HRESULT STDMETHODCALLTYPE ExceptionCatcherLeave() override;
    HRESULT STDMETHODCALLTYPE COMClassicVTableCreated(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable, ULONG cSlots) override;
    HRESULT STDMETHODCALLTYPE COMClassicVTableDestroyed(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable) override;
    HRESULT STDMETHODCALLTYPE ExceptionCLRCatcherFound() override;
    HRESULT STDMETHODCALLTYPE ExceptionCLRCatcherExecute() override;
    HRESULT STDMETHODCALLTYPE ThreadNameChanged(ThreadID threadId, ULONG cchName, WCHAR name[]) override;
    HRESULT STDMETHODCALLTYPE GarbageCollectionStarted(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason) override;
    HRESULT STDMETHODCALLTYPE SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[]) override;
    HRESULT STDMETHODCALLTYPE GarbageCollectionFinished() override;
    HRESULT STDMETHODCALLTYPE FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID) override;
    HRESULT STDMETHODCALLTYPE RootReferences2(ULONG cRootRefs, ObjectID rootRefIds[], COR_PRF_GC_ROOT_KIND rootKinds[], COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIds[]) override;
    HRESULT STDMETHODCALLTYPE HandleCreated(GCHandleID handleId, ObjectID initialObjectId) override;
    HRESULT STDMETHODCALLTYPE HandleDestroyed(GCHandleID handleId) override;
    HRESULT STDMETHODCALLTYPE InitializeForAttach(IUnknown* pCorProfilerInfoUnk, void* pvClientData, UINT cbClientData) override;
    HRESULT STDMETHODCALLTYPE ProfilerAttachComplete() override;
    HRESULT STDMETHODCALLTYPE ProfilerDetachSucceeded() override;
    HRESULT STDMETHODCALLTYPE ReJITCompilationStarted(FunctionID functionId, ReJITID rejitId, BOOL fIsSafeToBlock) override;
    HRESULT STDMETHODCALLTYPE GetReJITParameters(ModuleID moduleId, mdMethodDef methodId, ICorProfilerFunctionControl* pFunctionControl) override;
    HRESULT STDMETHODCALLTYPE ReJITCompilationFinished(FunctionID functionId, ReJITID rejitId, HRESULT hrStatus, BOOL fIsSafeToBlock) override;
    HRESULT STDMETHODCALLTYPE ReJITError(ModuleID moduleId, mdMethodDef methodId, FunctionID functionId, HRESULT hrStatus) override;
    HRESULT STDMETHODCALLTYPE MovedReferences2(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], SIZE_T cObjectIDRangeLength[]) override;
    HRESULT STDMETHODCALLTYPE SurvivingReferences2(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], SIZE_T cObjectIDRangeLength[]) override;
    HRESULT STDMETHODCALLTYPE ConditionalWeakTableElementReferences(ULONG cRootRefs, ObjectID keyRefIds[], ObjectID valueRefIds[], GCHandleID rootIds[]) override;
    HRESULT STDMETHODCALLTYPE GetAssemblyReferences(const WCHAR* wszAssemblyPath, ICorProfilerAssemblyReferenceProvider* pAsmRefProvider) override;
    HRESULT STDMETHODCALLTYPE ModuleInMemorySymbolsUpdated(ModuleID moduleId) override;

    HRESULT STDMETHODCALLTYPE DynamicMethodJITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock, LPCBYTE ilHeader, ULONG cbILHeader) override;
    HRESULT STDMETHODCALLTYPE DynamicMethodJITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock) override;

    void AddMethodParameters(util::wstring assemblyName, util::wstring typeName, util::wstring methodName, int methodParametersCount, std::vector<util::wstring> parameters);
    void AddMethodVariables(util::wstring assemblyName, util::wstring typeName, util::wstring methodName, int methodParametersCount, std::vector<util::wstring> variables);
    void AddDebuggerOffset(util::wstring assemblyName, util::wstring typeName, util::wstring methodName, int methodParametersCount, int offset);

    void StartDebugger();

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
        if (riid == __uuidof(ICorProfilerCallback8) ||
            riid == __uuidof(ICorProfilerCallback7) ||
            riid == __uuidof(ICorProfilerCallback6) ||
            riid == __uuidof(ICorProfilerCallback5) ||
            riid == __uuidof(ICorProfilerCallback4) ||
            riid == __uuidof(ICorProfilerCallback3) ||
            riid == __uuidof(ICorProfilerCallback2) ||
            riid == __uuidof(ICorProfilerCallback) ||
            riid == IID_IUnknown)
        {
            *ppvObject = this;
            this->AddRef();
            return S_OK;
        }

        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef(void) override
    {
        return std::atomic_fetch_add(&this->refCount, 1) + 1;
    }

    ULONG STDMETHODCALLTYPE Release(void) override
    {
        int count = std::atomic_fetch_sub(&this->refCount, 1) - 1;

        if (count <= 0)
        {
            delete this;
        }

        return count;
    }

    friend class MethodRewriter;
};

extern CorProfiler* instance;