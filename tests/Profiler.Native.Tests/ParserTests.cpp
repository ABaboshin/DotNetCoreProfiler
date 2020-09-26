#include "pch.h"
#include "info/parser.h"

class ParserTests : public ::testing::Test {};

TEST_F(ParserTests, ItDetectsIsVoid)
{
	EXPECT_TRUE(info::IsVoid({ ELEMENT_TYPE_VOID }));
}

TEST_F(ParserTests, ItDetectsIsntVoid)
{
	EXPECT_TRUE(!info::IsVoid({ ELEMENT_TYPE_VAR, ELEMENT_TYPE_MVAR }));
}

TEST_F(ParserTests, ItDoesntParseNotVoid2)
{
	EXPECT_TRUE(!info::IsVoid({ ELEMENT_TYPE_VAR }));
}

TEST_F(ParserTests, ItParsesNumber3)
{
	ULONG number;

	std::vector<BYTE> data = { 3 };
	auto iter = data.begin();
	EXPECT_TRUE(info::ParseNumber(iter, number));
	EXPECT_EQ(3, number);
}

// TODO parse optional custom mods

// TODO do it for all other build-in types
TEST_F(ParserTests, ItParsesTypeVoid)
{
	std::vector<BYTE> data = { ELEMENT_TYPE_VOID };
	auto iter = data.begin();
	EXPECT_TRUE(info::ParseType(iter));
}

TEST_F(ParserTests, ItParsesTypePtrInt)
{
	std::vector<BYTE> data = { ELEMENT_TYPE_PTR, ELEMENT_TYPE_I4 };
	auto iter = data.begin();
	EXPECT_TRUE(info::ParseType(iter));
}

//TODO ELEMENT_TYPE_VALUETYPE
//TODO ELEMENT_TYPE_CLASS

// TODO test parsing other arrays
TEST_F(ParserTests, ItParses_ELEMENT_TYPE_ARRAY)
{
	std::vector<BYTE> data = { ELEMENT_TYPE_ARRAY, ELEMENT_TYPE_I4, 1, 1, 1, 1, 1 };
	auto iter = data.begin();
	EXPECT_TRUE(info::ParseType(iter));
}

TEST_F(ParserTests, ItParses_ELEMENT_TYPE_SZARRAY)
{
	std::vector<BYTE> data = { ELEMENT_TYPE_SZARRAY, ELEMENT_TYPE_I4 };
	auto iter = data.begin();
	EXPECT_TRUE(info::ParseType(iter));
}

TEST_F(ParserTests, ItParses_ELEMENT_TYPE_VAR)
{
	std::vector<BYTE> data = { ELEMENT_TYPE_VAR, 1 };
	auto iter = data.begin();
	EXPECT_TRUE(info::ParseType(iter));
}

TEST_F(ParserTests, ItParses_ELEMENT_TYPE_MVAR)
{
	std::vector<BYTE> data = { ELEMENT_TYPE_MVAR, 1 };
	auto iter = data.begin();
	EXPECT_TRUE(info::ParseType(iter));
}

//TODO ELEMENT_TYPE_GENERICINST

TEST_F(ParserTests, ItParses_ELEMENT_TYPE_FNPTR_generic)
{
	std::vector<BYTE> data = { ELEMENT_TYPE_FNPTR, IMAGE_CEE_CS_CALLCONV_GENERIC, 1, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_I4 };
	auto iter = data.begin();
	EXPECT_TRUE(info::ParseType(iter));
}

TEST_F(ParserTests, ItParses_ELEMENT_TYPE_FNPTR_hasthis)
{
	std::vector<BYTE> data = { ELEMENT_TYPE_FNPTR, IMAGE_CEE_CS_CALLCONV_HASTHIS, 0, ELEMENT_TYPE_I4 };
	auto iter = data.begin();
	EXPECT_TRUE(info::ParseType(iter));
}

//TODO ParseParam
//TODO ParseRetType