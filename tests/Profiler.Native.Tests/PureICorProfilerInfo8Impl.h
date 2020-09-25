#pragma once

class PureICorProfilerInfo8Impl : public ::ICorProfilerInfo8
{
	// Inherited via ICorProfilerInfo8
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override
	{
		return E_NOTIMPL;
	}
	virtual ULONG __stdcall AddRef(void) override
	{
		return 0;
	}
	virtual ULONG __stdcall Release(void) override
	{
		return 0;
	}
	virtual HRESULT __stdcall GetClassFromObject(ObjectID objectId, ClassID* pClassId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetClassFromToken(ModuleID moduleId, mdTypeDef typeDef, ClassID* pClassId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetCodeInfo(FunctionID functionId, LPCBYTE* pStart, ULONG* pcSize) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetEventMask(DWORD* pdwEvents) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetFunctionFromIP(LPCBYTE ip, FunctionID* pFunctionId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetFunctionFromToken(ModuleID moduleId, mdToken token, FunctionID* pFunctionId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetHandleFromThread(ThreadID threadId, HANDLE* phThread) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetObjectSize(ObjectID objectId, ULONG* pcSize) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall IsArrayClass(ClassID classId, CorElementType* pBaseElemType, ClassID* pBaseClassId, ULONG* pcRank) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetThreadInfo(ThreadID threadId, DWORD* pdwWin32ThreadId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetCurrentThreadID(ThreadID* pThreadId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetClassIDInfo(ClassID classId, ModuleID* pModuleId, mdTypeDef* pTypeDefToken) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetFunctionInfo(FunctionID functionId, ClassID* pClassId, ModuleID* pModuleId, mdToken* pToken) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SetEventMask(DWORD dwEvents) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SetEnterLeaveFunctionHooks(FunctionEnter* pFuncEnter, FunctionLeave* pFuncLeave, FunctionTailcall* pFuncTailcall) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SetFunctionIDMapper(FunctionIDMapper* pFunc) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetTokenAndMetaDataFromFunction(FunctionID functionId, REFIID riid, IUnknown** ppImport, mdToken* pToken) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetModuleInfo(ModuleID moduleId, LPCBYTE* ppBaseLoadAddress, ULONG cchName, ULONG* pcchName, WCHAR szName[], AssemblyID* pAssemblyId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetModuleMetaData(ModuleID moduleId, DWORD dwOpenFlags, REFIID riid, IUnknown** ppOut) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetILFunctionBody(ModuleID moduleId, mdMethodDef methodId, LPCBYTE* ppMethodHeader, ULONG* pcbMethodSize) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetILFunctionBodyAllocator(ModuleID moduleId, IMethodMalloc** ppMalloc) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SetILFunctionBody(ModuleID moduleId, mdMethodDef methodid, LPCBYTE pbNewILMethodHeader) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetAppDomainInfo(AppDomainID appDomainId, ULONG cchName, ULONG* pcchName, WCHAR szName[], ProcessID* pProcessId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetAssemblyInfo(AssemblyID assemblyId, ULONG cchName, ULONG* pcchName, WCHAR szName[], AppDomainID* pAppDomainId, ModuleID* pModuleId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SetFunctionReJIT(FunctionID functionId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ForceGC(void) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SetILInstrumentedCodeMap(FunctionID functionId, BOOL fStartJit, ULONG cILMapEntries, COR_IL_MAP rgILMapEntries[]) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetInprocInspectionInterface(IUnknown** ppicd) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetInprocInspectionIThisThread(IUnknown** ppicd) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetThreadContext(ThreadID threadId, ContextID* pContextId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall BeginInprocDebugging(BOOL fThisThreadOnly, DWORD* pdwProfilerContext) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EndInprocDebugging(DWORD dwProfilerContext) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetILToNativeMapping(FunctionID functionId, ULONG32 cMap, ULONG32* pcMap, COR_DEBUG_IL_TO_NATIVE_MAP map[]) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall DoStackSnapshot(ThreadID thread, StackSnapshotCallback* callback, ULONG32 infoFlags, void* clientData, BYTE context[], ULONG32 contextSize) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SetEnterLeaveFunctionHooks2(FunctionEnter2* pFuncEnter, FunctionLeave2* pFuncLeave, FunctionTailcall2* pFuncTailcall) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetFunctionInfo2(FunctionID funcId, COR_PRF_FRAME_INFO frameInfo, ClassID* pClassId, ModuleID* pModuleId, mdToken* pToken, ULONG32 cTypeArgs, ULONG32* pcTypeArgs, ClassID typeArgs[]) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetStringLayout(ULONG* pBufferLengthOffset, ULONG* pStringLengthOffset, ULONG* pBufferOffset) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetClassLayout(ClassID classID, COR_FIELD_OFFSET rFieldOffset[], ULONG cFieldOffset, ULONG* pcFieldOffset, ULONG* pulClassSize) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetClassIDInfo2(ClassID classId, ModuleID* pModuleId, mdTypeDef* pTypeDefToken, ClassID* pParentClassId, ULONG32 cNumTypeArgs, ULONG32* pcNumTypeArgs, ClassID typeArgs[]) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetCodeInfo2(FunctionID functionID, ULONG32 cCodeInfos, ULONG32* pcCodeInfos, COR_PRF_CODE_INFO codeInfos[]) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetClassFromTokenAndTypeArgs(ModuleID moduleID, mdTypeDef typeDef, ULONG32 cTypeArgs, ClassID typeArgs[], ClassID* pClassID) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetFunctionFromTokenAndTypeArgs(ModuleID moduleID, mdMethodDef funcDef, ClassID classId, ULONG32 cTypeArgs, ClassID typeArgs[], FunctionID* pFunctionID) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumModuleFrozenObjects(ModuleID moduleID, ICorProfilerObjectEnum** ppEnum) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetArrayObjectInfo(ObjectID objectId, ULONG32 cDimensions, ULONG32 pDimensionSizes[], int pDimensionLowerBounds[], BYTE** ppData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetBoxClassLayout(ClassID classId, ULONG32* pBufferOffset) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetThreadAppDomain(ThreadID threadId, AppDomainID* pAppDomainId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetRVAStaticAddress(ClassID classId, mdFieldDef fieldToken, void** ppAddress) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetAppDomainStaticAddress(ClassID classId, mdFieldDef fieldToken, AppDomainID appDomainId, void** ppAddress) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetThreadStaticAddress(ClassID classId, mdFieldDef fieldToken, ThreadID threadId, void** ppAddress) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetContextStaticAddress(ClassID classId, mdFieldDef fieldToken, ContextID contextId, void** ppAddress) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetStaticFieldInfo(ClassID classId, mdFieldDef fieldToken, COR_PRF_STATIC_TYPE* pFieldInfo) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetGenerationBounds(ULONG cObjectRanges, ULONG* pcObjectRanges, COR_PRF_GC_GENERATION_RANGE ranges[]) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetObjectGeneration(ObjectID objectId, COR_PRF_GC_GENERATION_RANGE* range) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetNotifiedExceptionClauseInfo(COR_PRF_EX_CLAUSE_INFO* pinfo) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumJITedFunctions(ICorProfilerFunctionEnum** ppEnum) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall RequestProfilerDetach(DWORD dwExpectedCompletionMilliseconds) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SetFunctionIDMapper2(FunctionIDMapper2* pFunc, void* clientData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetStringLayout2(ULONG* pStringLengthOffset, ULONG* pBufferOffset) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SetEnterLeaveFunctionHooks3(FunctionEnter3* pFuncEnter3, FunctionLeave3* pFuncLeave3, FunctionTailcall3* pFuncTailcall3) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SetEnterLeaveFunctionHooks3WithInfo(FunctionEnter3WithInfo* pFuncEnter3WithInfo, FunctionLeave3WithInfo* pFuncLeave3WithInfo, FunctionTailcall3WithInfo* pFuncTailcall3WithInfo) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetFunctionEnter3Info(FunctionID functionId, COR_PRF_ELT_INFO eltInfo, COR_PRF_FRAME_INFO* pFrameInfo, ULONG* pcbArgumentInfo, COR_PRF_FUNCTION_ARGUMENT_INFO* pArgumentInfo) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetFunctionLeave3Info(FunctionID functionId, COR_PRF_ELT_INFO eltInfo, COR_PRF_FRAME_INFO* pFrameInfo, COR_PRF_FUNCTION_ARGUMENT_RANGE* pRetvalRange) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetFunctionTailcall3Info(FunctionID functionId, COR_PRF_ELT_INFO eltInfo, COR_PRF_FRAME_INFO* pFrameInfo) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumModules(ICorProfilerModuleEnum** ppEnum) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetRuntimeInformation(USHORT* pClrInstanceId, COR_PRF_RUNTIME_TYPE* pRuntimeType, USHORT* pMajorVersion, USHORT* pMinorVersion, USHORT* pBuildNumber, USHORT* pQFEVersion, ULONG cchVersionString, ULONG* pcchVersionString, WCHAR szVersionString[]) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetThreadStaticAddress2(ClassID classId, mdFieldDef fieldToken, AppDomainID appDomainId, ThreadID threadId, void** ppAddress) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetAppDomainsContainingModule(ModuleID moduleId, ULONG32 cAppDomainIds, ULONG32* pcAppDomainIds, AppDomainID appDomainIds[]) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetModuleInfo2(ModuleID moduleId, LPCBYTE* ppBaseLoadAddress, ULONG cchName, ULONG* pcchName, WCHAR szName[], AssemblyID* pAssemblyId, DWORD* pdwModuleFlags) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumThreads(ICorProfilerThreadEnum** ppEnum) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall InitializeCurrentThread(void) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall RequestReJIT(ULONG cFunctions, ModuleID moduleIds[], mdMethodDef methodIds[]) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall RequestRevert(ULONG cFunctions, ModuleID moduleIds[], mdMethodDef methodIds[], HRESULT status[]) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetCodeInfo3(FunctionID functionID, ReJITID reJitId, ULONG32 cCodeInfos, ULONG32* pcCodeInfos, COR_PRF_CODE_INFO codeInfos[]) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetFunctionFromIP2(LPCBYTE ip, FunctionID* pFunctionId, ReJITID* pReJitId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetReJITIDs(FunctionID functionId, ULONG cReJitIds, ULONG* pcReJitIds, ReJITID reJitIds[]) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetILToNativeMapping2(FunctionID functionId, ReJITID reJitId, ULONG32 cMap, ULONG32* pcMap, COR_DEBUG_IL_TO_NATIVE_MAP map[]) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumJITedFunctions2(ICorProfilerFunctionEnum** ppEnum) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetObjectSize2(ObjectID objectId, SIZE_T* pcSize) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetEventMask2(DWORD* pdwEventsLow, DWORD* pdwEventsHigh) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SetEventMask2(DWORD dwEventsLow, DWORD dwEventsHigh) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumNgenModuleMethodsInliningThisMethod(ModuleID inlinersModuleId, ModuleID inlineeModuleId, mdMethodDef inlineeMethodId, BOOL* incompleteData, ICorProfilerMethodEnum** ppEnum) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ApplyMetaData(ModuleID moduleId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetInMemorySymbolsLength(ModuleID moduleId, DWORD* pCountSymbolBytes) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ReadInMemorySymbols(ModuleID moduleId, DWORD symbolsReadOffset, BYTE* pSymbolBytes, DWORD countSymbolBytes, DWORD* pCountSymbolBytesRead) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall IsFunctionDynamic(FunctionID functionId, BOOL* isDynamic) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetFunctionFromIP3(LPCBYTE ip, FunctionID* functionId, ReJITID* pReJitId) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetDynamicFunctionInfo(FunctionID functionId, ModuleID* moduleId, PCCOR_SIGNATURE* ppvSig, ULONG* pbSig, ULONG cchName, ULONG* pcchName, WCHAR wszName[]) override
	{
		return E_NOTIMPL;
	}
};