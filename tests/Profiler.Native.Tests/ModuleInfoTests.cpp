#include "pch.h"
#include "info/ModuleInfo.h"
#include "mocks.h"
#include "PureICorProfilerInfo8Impl.h"

using namespace util;

class ModuleICorProfilerInfo8Impl : public ::PureICorProfilerInfo8Impl
{
private:
	ModuleID moduleId;
	AssemblyID assemblyId;
	AppDomainID appDomainId;
	::util::wstring moduleName;
	::util::wstring assemblyName;
	::util::wstring appDomainName;

public:
	ModuleICorProfilerInfo8Impl(ModuleID moduleId, ::util::wstring moduleName, AssemblyID assemblyId, AppDomainID appDomainId, const ::util::wstring& assemblyName, const ::util::wstring& appDomainName) :
		moduleId(moduleId), moduleName(moduleName), assemblyId(assemblyId), appDomainId(appDomainId), assemblyName(assemblyName), appDomainName(appDomainName) { }

	virtual HRESULT __stdcall GetAssemblyInfo(AssemblyID assemblyId, ULONG cchName, ULONG* pcchName, WCHAR szName[], AppDomainID* pAppDomainId, ModuleID* pModuleId) override
	{
		if (assemblyId == this->assemblyId)
		{
			*pcchName = assemblyName.size();
			wcsncpy_s(szName, cchName, assemblyName.c_str(), assemblyName.size());
			*pAppDomainId = appDomainId;

			return S_OK;
		}

		return E_NOTIMPL;
	}

	virtual HRESULT __stdcall GetAppDomainInfo(AppDomainID appDomainId, ULONG cchName, ULONG* pcchName, WCHAR szName[], ProcessID* pProcessId) override
	{
		if (appDomainId == this->appDomainId)
		{
			*pcchName = appDomainName.size();
			wcsncpy_s(szName, cchName, appDomainName.c_str(), appDomainName.size());

			return S_OK;
		}

		return E_NOTIMPL;
	}

	virtual HRESULT __stdcall GetModuleInfo2(ModuleID moduleId, LPCBYTE* ppBaseLoadAddress, ULONG cchName, ULONG* pcchName, WCHAR szName[], AssemblyID* pAssemblyId, DWORD* pdwModuleFlags) override
	{
		if (moduleId == this->moduleId)
		{
			*pcchName = moduleName.size();
			wcsncpy_s(szName, cchName, moduleName.c_str(), moduleName.size());

			*pAssemblyId = assemblyId;

			return S_OK;
		}

		return E_NOTIMPL;
	}
};

class ModuleInfoTests : public ::testing::Test {
protected:
	void SetUp() override {
		moduleId = 1;
		assemblyId = 1;
		appDomainId = 1;
		moduleName = "module"_W;
		assemblyName = "assembly"_W;
		appDomainName = "appDomain"_W;

		info = new ModuleICorProfilerInfo8Impl(moduleId, moduleName, assemblyId, appDomainId, assemblyName, appDomainName);
	}

	void TearDown() override {
		delete info;
	}

	ICorProfilerInfo8* info;

	ModuleID moduleId;
	AssemblyID assemblyId;
	AppDomainID appDomainId;
	::util::wstring moduleName;
	::util::wstring assemblyName;
	::util::wstring appDomainName;
};

TEST_F(ModuleInfoTests, ItGetsModuleInfo)
{
	auto mi = info::ModuleInfo::GetModuleInfo(info, moduleId);

	EXPECT_EQ(moduleId, mi.id);
	EXPECT_EQ(moduleName, mi.path);
}

TEST_F(ModuleInfoTests, ItFailedForUnknownModule)
{
	auto mi = info::ModuleInfo::GetModuleInfo(info, 300);

	EXPECT_NE(moduleId, mi.id);
}