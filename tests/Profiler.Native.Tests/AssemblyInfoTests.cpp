#include "pch.h"
#include "info/AssemblyInfo.h"
#include "mocks.h"
#include "PureICorProfilerInfo8Impl.h"

using namespace util;

class AssemblyICorProfilerInfo8Impl : public ::PureICorProfilerInfo8Impl
{
private:
	AssemblyID assemblyId;
	AppDomainID appDomainId;
	::util::wstring assemblyName;
	::util::wstring appDomainName;

public:
	AssemblyICorProfilerInfo8Impl(AssemblyID assemblyId, AppDomainID appDomainId, const ::util::wstring& assemblyName, const ::util::wstring& appDomainName) :
		assemblyId(assemblyId), appDomainId(appDomainId), assemblyName(assemblyName), appDomainName(appDomainName) { }

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
};

class AssemblyInfoTests : public ::testing::Test {
protected:
	void SetUp() override {
		assemblyId = 1;
		appDomainId = 1;
		assemblyName = "assembly"_W;
		appDomainName = "appDomain"_W;
		info = new AssemblyICorProfilerInfo8Impl(assemblyId, appDomainId, assemblyName, appDomainName);
	}

	void TearDown() override {
		delete info;
	}

	ICorProfilerInfo8* info;
	AssemblyID assemblyId;
	AppDomainID appDomainId;
	::util::wstring assemblyName;
	::util::wstring appDomainName;
};

TEST_F(AssemblyInfoTests, ItGetsAssemblyInfo)
{
	auto ai = info::AssemblyInfo::GetAssemblyInfo(info, assemblyId);

	EXPECT_EQ(assemblyId, ai.id);
	EXPECT_EQ(appDomainId, ai.appDomainId);
	EXPECT_EQ(assemblyName, ai.name);
	EXPECT_EQ(appDomainName, ai.appDomainName);
}

TEST_F(AssemblyInfoTests, ItFailedForUnknownAssembly)
{
	auto ai = info::AssemblyInfo::GetAssemblyInfo(info, 300);

	EXPECT_NE(assemblyId, ai.id);
}