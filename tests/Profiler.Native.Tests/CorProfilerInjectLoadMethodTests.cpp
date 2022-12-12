#include "pch.h"
#include "profiler/CorProfiler.h"
#include "PureICorProfilerInfo8Impl.h"

class CorProfilerInjectLoadMethodTests : public ::testing::Test {};

class IMetaDataImport2Mock : public IMetaDataImport2
{
	// Inherited via IMetaDataImport2
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override
	{
		*ppvObject = this;
		return S_OK;
	}
	virtual ULONG __stdcall AddRef(void) override
	{
		return 0;
	}
	virtual ULONG __stdcall Release(void) override
	{
		return 0;
	}
	virtual void __stdcall CloseEnum(HCORENUM hEnum) override
	{
	}
	virtual HRESULT __stdcall CountEnum(HCORENUM hEnum, ULONG* pulCount) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ResetEnum(HCORENUM hEnum, ULONG ulPos) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumTypeDefs(HCORENUM* phEnum, mdTypeDef rTypeDefs[], ULONG cMax, ULONG* pcTypeDefs) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumInterfaceImpls(HCORENUM* phEnum, mdTypeDef td, mdInterfaceImpl rImpls[], ULONG cMax, ULONG* pcImpls) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumTypeRefs(HCORENUM* phEnum, mdTypeRef rTypeRefs[], ULONG cMax, ULONG* pcTypeRefs) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall FindTypeDefByName(LPCWSTR szTypeDef, mdToken tkEnclosingClass, mdTypeDef* ptd) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetScopeProps(LPWSTR szName, ULONG cchName, ULONG* pchName, GUID* pmvid) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetModuleFromScope(mdModule* pmd) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetTypeDefProps(mdTypeDef td, LPWSTR szTypeDef, ULONG cchTypeDef, ULONG* pchTypeDef, DWORD* pdwTypeDefFlags, mdToken* ptkExtends) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetInterfaceImplProps(mdInterfaceImpl iiImpl, mdTypeDef* pClass, mdToken* ptkIface) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetTypeRefProps(mdTypeRef tr, mdToken* ptkResolutionScope, LPWSTR szName, ULONG cchName, ULONG* pchName) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ResolveTypeRef(mdTypeRef tr, REFIID riid, IUnknown** ppIScope, mdTypeDef* ptd) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumMembers(HCORENUM* phEnum, mdTypeDef cl, mdToken rMembers[], ULONG cMax, ULONG* pcTokens) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumMembersWithName(HCORENUM* phEnum, mdTypeDef cl, LPCWSTR szName, mdToken rMembers[], ULONG cMax, ULONG* pcTokens) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumMethods(HCORENUM* phEnum, mdTypeDef cl, mdMethodDef rMethods[], ULONG cMax, ULONG* pcTokens) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumMethodsWithName(HCORENUM* phEnum, mdTypeDef cl, LPCWSTR szName, mdMethodDef rMethods[], ULONG cMax, ULONG* pcTokens) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumFields(HCORENUM* phEnum, mdTypeDef cl, mdFieldDef rFields[], ULONG cMax, ULONG* pcTokens) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumFieldsWithName(HCORENUM* phEnum, mdTypeDef cl, LPCWSTR szName, mdFieldDef rFields[], ULONG cMax, ULONG* pcTokens) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumParams(HCORENUM* phEnum, mdMethodDef mb, mdParamDef rParams[], ULONG cMax, ULONG* pcTokens) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumMemberRefs(HCORENUM* phEnum, mdToken tkParent, mdMemberRef rMemberRefs[], ULONG cMax, ULONG* pcTokens) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumMethodImpls(HCORENUM* phEnum, mdTypeDef td, mdToken rMethodBody[], mdToken rMethodDecl[], ULONG cMax, ULONG* pcTokens) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumPermissionSets(HCORENUM* phEnum, mdToken tk, DWORD dwActions, mdPermission rPermission[], ULONG cMax, ULONG* pcTokens) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall FindMember(mdTypeDef td, LPCWSTR szName, PCCOR_SIGNATURE pvSigBlob, ULONG cbSigBlob, mdToken* pmb) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall FindMethod(mdTypeDef td, LPCWSTR szName, PCCOR_SIGNATURE pvSigBlob, ULONG cbSigBlob, mdMethodDef* pmb) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall FindField(mdTypeDef td, LPCWSTR szName, PCCOR_SIGNATURE pvSigBlob, ULONG cbSigBlob, mdFieldDef* pmb) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall FindMemberRef(mdTypeRef td, LPCWSTR szName, PCCOR_SIGNATURE pvSigBlob, ULONG cbSigBlob, mdMemberRef* pmr) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetMethodProps(mdMethodDef mb, mdTypeDef* pClass, LPWSTR szMethod, ULONG cchMethod, ULONG* pchMethod, DWORD* pdwAttr, PCCOR_SIGNATURE* ppvSigBlob, ULONG* pcbSigBlob, ULONG* pulCodeRVA, DWORD* pdwImplFlags) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetMemberRefProps(mdMemberRef mr, mdToken* ptk, LPWSTR szMember, ULONG cchMember, ULONG* pchMember, PCCOR_SIGNATURE* ppvSigBlob, ULONG* pbSig) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumProperties(HCORENUM* phEnum, mdTypeDef td, mdProperty rProperties[], ULONG cMax, ULONG* pcProperties) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumEvents(HCORENUM* phEnum, mdTypeDef td, mdEvent rEvents[], ULONG cMax, ULONG* pcEvents) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetEventProps(mdEvent ev, mdTypeDef* pClass, LPCWSTR szEvent, ULONG cchEvent, ULONG* pchEvent, DWORD* pdwEventFlags, mdToken* ptkEventType, mdMethodDef* pmdAddOn, mdMethodDef* pmdRemoveOn, mdMethodDef* pmdFire, mdMethodDef rmdOtherMethod[], ULONG cMax, ULONG* pcOtherMethod) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumMethodSemantics(HCORENUM* phEnum, mdMethodDef mb, mdToken rEventProp[], ULONG cMax, ULONG* pcEventProp) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetMethodSemantics(mdMethodDef mb, mdToken tkEventProp, DWORD* pdwSemanticsFlags) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetClassLayout(mdTypeDef td, DWORD* pdwPackSize, COR_FIELD_OFFSET rFieldOffset[], ULONG cMax, ULONG* pcFieldOffset, ULONG* pulClassSize) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetFieldMarshal(mdToken tk, PCCOR_SIGNATURE* ppvNativeType, ULONG* pcbNativeType) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetRVA(mdToken tk, ULONG* pulCodeRVA, DWORD* pdwImplFlags) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetPermissionSetProps(mdPermission pm, DWORD* pdwAction, void const** ppvPermission, ULONG* pcbPermission) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetSigFromToken(mdSignature mdSig, PCCOR_SIGNATURE* ppvSig, ULONG* pcbSig) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetModuleRefProps(mdModuleRef mur, LPWSTR szName, ULONG cchName, ULONG* pchName) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumModuleRefs(HCORENUM* phEnum, mdModuleRef rModuleRefs[], ULONG cmax, ULONG* pcModuleRefs) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetTypeSpecFromToken(mdTypeSpec typespec, PCCOR_SIGNATURE* ppvSig, ULONG* pcbSig) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetNameFromToken(mdToken tk, MDUTF8CSTR* pszUtf8NamePtr) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumUnresolvedMethods(HCORENUM* phEnum, mdToken rMethods[], ULONG cMax, ULONG* pcTokens) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetUserString(mdString stk, LPWSTR szString, ULONG cchString, ULONG* pchString) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetPinvokeMap(mdToken tk, DWORD* pdwMappingFlags, LPWSTR szImportName, ULONG cchImportName, ULONG* pchImportName, mdModuleRef* pmrImportDLL) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumSignatures(HCORENUM* phEnum, mdSignature rSignatures[], ULONG cmax, ULONG* pcSignatures) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumTypeSpecs(HCORENUM* phEnum, mdTypeSpec rTypeSpecs[], ULONG cmax, ULONG* pcTypeSpecs) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumUserStrings(HCORENUM* phEnum, mdString rStrings[], ULONG cmax, ULONG* pcStrings) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetParamForMethodIndex(mdMethodDef md, ULONG ulParamSeq, mdParamDef* ppd) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumCustomAttributes(HCORENUM* phEnum, mdToken tk, mdToken tkType, mdCustomAttribute rCustomAttributes[], ULONG cMax, ULONG* pcCustomAttributes) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetCustomAttributeProps(mdCustomAttribute cv, mdToken* ptkObj, mdToken* ptkType, void const** ppBlob, ULONG* pcbSize) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall FindTypeRef(mdToken tkResolutionScope, LPCWSTR szName, mdTypeRef* ptr) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetMemberProps(mdToken mb, mdTypeDef* pClass, LPWSTR szMember, ULONG cchMember, ULONG* pchMember, DWORD* pdwAttr, PCCOR_SIGNATURE* ppvSigBlob, ULONG* pcbSigBlob, ULONG* pulCodeRVA, DWORD* pdwImplFlags, DWORD* pdwCPlusTypeFlag, UVCP_CONSTANT* ppValue, ULONG* pcchValue) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetFieldProps(mdFieldDef mb, mdTypeDef* pClass, LPWSTR szField, ULONG cchField, ULONG* pchField, DWORD* pdwAttr, PCCOR_SIGNATURE* ppvSigBlob, ULONG* pcbSigBlob, DWORD* pdwCPlusTypeFlag, UVCP_CONSTANT* ppValue, ULONG* pcchValue) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetPropertyProps(mdProperty prop, mdTypeDef* pClass, LPCWSTR szProperty, ULONG cchProperty, ULONG* pchProperty, DWORD* pdwPropFlags, PCCOR_SIGNATURE* ppvSig, ULONG* pbSig, DWORD* pdwCPlusTypeFlag, UVCP_CONSTANT* ppDefaultValue, ULONG* pcchDefaultValue, mdMethodDef* pmdSetter, mdMethodDef* pmdGetter, mdMethodDef rmdOtherMethod[], ULONG cMax, ULONG* pcOtherMethod) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetParamProps(mdParamDef tk, mdMethodDef* pmd, ULONG* pulSequence, LPWSTR szName, ULONG cchName, ULONG* pchName, DWORD* pdwAttr, DWORD* pdwCPlusTypeFlag, UVCP_CONSTANT* ppValue, ULONG* pcchValue) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetCustomAttributeByName(mdToken tkObj, LPCWSTR szName, const void** ppData, ULONG* pcbData) override
	{
		return E_NOTIMPL;
	}
	virtual BOOL __stdcall IsValidToken(mdToken tk) override
	{
		return 0;
	}
	virtual HRESULT __stdcall GetNestedClassProps(mdTypeDef tdNestedClass, mdTypeDef* ptdEnclosingClass) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetNativeCallConvFromSig(void const* pvSig, ULONG cbSig, ULONG* pCallConv) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall IsGlobal(mdToken pd, int* pbGlobal) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumGenericParams(HCORENUM* phEnum, mdToken tk, mdGenericParam rGenericParams[], ULONG cMax, ULONG* pcGenericParams) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetGenericParamProps(mdGenericParam gp, ULONG* pulParamSeq, DWORD* pdwParamFlags, mdToken* ptOwner, DWORD* reserved, LPWSTR wzname, ULONG cchName, ULONG* pchName) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetMethodSpecProps(mdMethodSpec mi, mdToken* tkParent, PCCOR_SIGNATURE* ppvSigBlob, ULONG* pcbSigBlob) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumGenericParamConstraints(HCORENUM* phEnum, mdGenericParam tk, mdGenericParamConstraint rGenericParamConstraints[], ULONG cMax, ULONG* pcGenericParamConstraints) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetGenericParamConstraintProps(mdGenericParamConstraint gpc, mdGenericParam* ptGenericParam, mdToken* ptkConstraintType) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetPEKind(DWORD* pdwPEKind, DWORD* pdwMAchine) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetVersionString(LPWSTR pwzBuf, DWORD ccBufSize, DWORD* pccBufSize) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EnumMethodSpecs(HCORENUM* phEnum, mdToken tk, mdMethodSpec rMethodSpecs[], ULONG cMax, ULONG* pcMethodSpecs) override
	{
		return E_NOTIMPL;
	}
};

class ICorProfilerInfo8Mock : public ::PureICorProfilerInfo8Impl
{
	virtual HRESULT __stdcall GetFunctionInfo(FunctionID functionId, ClassID* pClassId, ModuleID* pModuleId, mdToken* pToken) override
	{
		*pModuleId = 0;
		return S_OK;
	}

	virtual HRESULT __stdcall GetModuleMetaData(ModuleID moduleId, DWORD dwOpenFlags, REFIID riid, IUnknown** ppOut) override
	{
		*ppOut = new IMetaDataImport2Mock();
		return S_OK;
	}
};

class ILRewriterMock : public rewriter::ILRewriter
{
public:
	ILRewriterMock(ICorProfilerInfo* pICorProfilerInfo, ICorProfilerFunctionControl* pICorProfilerFunctionControl, ModuleID moduleID, mdToken tkMethod) :
		rewriter::ILRewriter(pICorProfilerInfo, pICorProfilerFunctionControl, moduleID, tkMethod) {}

	HRESULT Import() override
	{
		return S_OK;
	}
};

class CorProfilerMock : public CorProfiler
{
public:
	int InjectLoadMethodCallCount = 0;
protected:
	HRESULT InjectLoadMethod(ModuleID moduleId, rewriter::ILRewriter& rewriter) override
	{
		InjectLoadMethodCallCount++;
		return S_OK;
	}

public:
	CorProfilerMock()
	{
		this->corProfilerInfo = new ICorProfilerInfo8Mock();
	}

	void SetEnabled(ModuleID id)
	{
		enabledModules.push_back(id);
	}

	void SetConfiguration(const configuration::Configuration configuration)
	{
		this->configuration = configuration;
	}

	rewriter::ILRewriter* CreateILRewriter(ICorProfilerFunctionControl* pICorProfilerFunctionControl, ModuleID moduleId, mdToken functionToken) override
	{
		return new ILRewriterMock(this->corProfilerInfo, nullptr, moduleId, functionToken);
	}
};

TEST_F(CorProfilerInjectLoadMethodTests, ItCallsInjectLoadMethodAtFirstCallInAppDomain)
{
	auto mock = new CorProfilerMock();
	mock->SetEnabled(0);

	mock->JITCompilationStarted(0, FALSE);
	EXPECT_EQ(mock->InjectLoadMethodCallCount, 1);
}

TEST_F(CorProfilerInjectLoadMethodTests, ItCallsInjectLoadMethodOnlyOnceInAppDomain)
{
	auto mock = new CorProfilerMock();
	mock->SetEnabled(0);

	mock->JITCompilationStarted(0, FALSE);
	mock->JITCompilationStarted(0, FALSE);
	EXPECT_EQ(mock->InjectLoadMethodCallCount, 1);
}

TEST_F(CorProfilerInjectLoadMethodTests, ItSkipsRewriting)
{
	std::stringstream s("{\"skipAssemblies\": [\"\"]}");

	auto cfg = ::configuration::Configuration::LoadFromStream(s);

	auto mock = new CorProfilerMock();
	mock->SetConfiguration(cfg);

	mock->JITCompilationStarted(0, FALSE);
	EXPECT_EQ(mock->InjectLoadMethodCallCount, 0);
}