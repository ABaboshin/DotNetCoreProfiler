#include <sstream>
#include <string>

#include "pch.h"
#include "configuration/Configuration.h"

class ConfigurationTest : public ::testing::Test {
protected:
	void SetUp() override {

	}

	void TearDown() override {

	}
};

TEST_F(ConfigurationTest, ItReads2Assemblies)
{
	std::stringstream s("{\"assemblies\": [\"lib1\", \"lib2\"]}");

	auto cfg = ::configuration::Configuration::LoadFromStream(s);

	EXPECT_EQ(2, cfg.Assemblies.size());
	EXPECT_EQ("lib1"_W, cfg.Assemblies[0]);
	EXPECT_EQ("lib2"_W, cfg.Assemblies[1]);
}

TEST_F(ConfigurationTest, ItReadsComposedInterceptor)
{
	std::stringstream s("{\"composedInterceptor\": {\"TypeName\": \"t1\", \"AssemblyName\": \"a1\"}}");

	auto cfg = ::configuration::Configuration::LoadFromStream(s);

	EXPECT_EQ("t1"_W, cfg.ComposedInterceptor.TypeName);
	EXPECT_EQ("a1"_W, cfg.ComposedInterceptor.AssemblyName);
}

TEST_F(ConfigurationTest, ItReadsInterceptorInterface)
{
	std::stringstream s("{\"interceptorInterface\": {\"TypeName\": \"t1\", \"AssemblyName\": \"a1\"}}");

	auto cfg = ::configuration::Configuration::LoadFromStream(s);

	EXPECT_EQ("t1"_W, cfg.InterceptorInterface.TypeName);
	EXPECT_EQ("a1"_W, cfg.InterceptorInterface.AssemblyName);
}

TEST_F(ConfigurationTest, ItReadsMethodFinderInterface)
{
	std::stringstream s("{\"methodFinderInterface\": {\"TypeName\": \"t1\", \"AssemblyName\": \"a1\"}}");

	auto cfg = ::configuration::Configuration::LoadFromStream(s);

	EXPECT_EQ("t1"_W, cfg.MethodFinderInterface.TypeName);
	EXPECT_EQ("a1"_W, cfg.MethodFinderInterface.AssemblyName);
}

TEST_F(ConfigurationTest, ItReads1MethodFinder)
{
	std::stringstream s("{\"methodFinders\": [{\"MethodFinder\": {\"TypeName\": \"t1\", \"AssemblyName\": \"a1\"}, \"Target\": {\"TypeName\": \"t1\", \"AssemblyName\": \"a1\", \"MethodName\": \"m1\", \"MethodParametersCount\": 1}}]}");

	auto cfg = ::configuration::Configuration::LoadFromStream(s);

	EXPECT_EQ(1, cfg.MethodFinders.size());
	EXPECT_EQ("a1"_W, cfg.MethodFinders[0].Finder.AssemblyName);
	EXPECT_EQ("t1"_W, cfg.MethodFinders[0].Finder.TypeName);
	EXPECT_EQ("a1"_W, cfg.MethodFinders[0].Target.AssemblyName);
	EXPECT_EQ("t1"_W, cfg.MethodFinders[0].Target.TypeName);
	EXPECT_EQ("m1"_W, cfg.MethodFinders[0].Target.MethodName);
	EXPECT_EQ(1, cfg.MethodFinders[0].Target.MethodParametersCount);
}

template<typename T>
::testing::AssertionResult UnorderedSetContains(const std::unordered_set<T>& set, const T& element)
{
	if (set.find(element) != set.end())
	{
		return ::testing::AssertionSuccess();
	}

	return ::testing::AssertionFailure();
}

TEST_F(ConfigurationTest, ItReads2SkipAssemblies)
{
	std::stringstream s("{\"skipAssemblies\": [\"lib1\", \"lib2\"]}");

	auto cfg = ::configuration::Configuration::LoadFromStream(s);

	EXPECT_EQ(2, cfg.SkipAssemblies.size());
	EXPECT_TRUE(UnorderedSetContains(cfg.SkipAssemblies, "lib1"_W));
	EXPECT_TRUE(UnorderedSetContains(cfg.SkipAssemblies, "lib2"_W));
}

TEST_F(ConfigurationTest, ItReads1StrictInterceptor)
{
	std::stringstream s("{\"strict\": [{\"IgnoreCallerAssemblies\": null, \"Interceptor\": {\"TypeName\": \"t1\", \"AssemblyName\": \"a1\"}, \"Target\": {\"TypeName\": \"t1\", \"AssemblyName\": \"a1\", \"MethodName\": \"m1\", \"MethodParametersCount\": 1}}]}");

	auto cfg = ::configuration::Configuration::LoadFromStream(s);

	EXPECT_EQ(1, cfg.StrictInterceptions.size());
	EXPECT_EQ(0, cfg.StrictInterceptions[0].IgnoreCallerAssemblies.size());
	EXPECT_EQ("a1"_W, cfg.StrictInterceptions[0].Interceptor.AssemblyName);
	EXPECT_EQ("t1"_W, cfg.StrictInterceptions[0].Interceptor.TypeName);
	EXPECT_EQ("a1"_W, cfg.StrictInterceptions[0].Target.AssemblyName);
	EXPECT_EQ("t1"_W, cfg.StrictInterceptions[0].Target.TypeName);
	EXPECT_EQ("m1"_W, cfg.StrictInterceptions[0].Target.MethodName);
	EXPECT_EQ(1, cfg.StrictInterceptions[0].Target.MethodParametersCount);
}