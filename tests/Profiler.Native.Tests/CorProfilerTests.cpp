#include "pch.h"
#include "profiler/CorProfiler.h"

class CorProfilerTests : public ::testing::Test {};

class CorProfilerMock : public CorProfiler
{
public:
	void SetConfiguration(const configuration::Configuration& configuration)
	{
		this->configuration = configuration;
	}
};

TEST_F(CorProfilerTests, Dummy)
{
	auto mock = new CorProfilerMock();
	//mock->SetConfiguration();
	EXPECT_TRUE(true);
}