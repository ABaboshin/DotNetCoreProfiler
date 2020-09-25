#include "pch.h"

class DummyTest : public ::testing::Test {
protected:
	void SetUp() override {

	}

	void TearDown() override {

	}
};

TEST_F(DummyTest, DummyTest1)
{
	EXPECT_EQ(1, 1);
}