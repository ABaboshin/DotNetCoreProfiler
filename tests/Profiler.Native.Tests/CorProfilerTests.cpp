#include "pch.h"
#include "profiler/CorProfiler.h"

class CorProfilerTests : public ::testing::Test {};

class CorProfilerMock : public CorProfiler
{
public:
	HRESULT STDMETHODCALLTYPE Initialize(IUnknown* pICorProfilerInfoUnk) override
	{
		return E_FAIL;
	}
};

TEST_F(CorProfilerTests, Dummy)
{
	auto mock = new CorProfilerMock();
	mock->Initialize(NULL);
	EXPECT_TRUE(true);
}