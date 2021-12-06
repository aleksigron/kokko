#include "Core/StringUtil.hpp"

#include <cstdint>

#include "doctest/doctest.h"

TEST_CASE("StringUtil.IntegerToHexadecimal")
{
	char buffer[17];

	uint32_t val32 = 0x01234567;
	kokko::IntegerToHexadecimal(val32, buffer);
	CHECK(buffer[0] == '0');
	CHECK(buffer[1] == '1');
	CHECK(buffer[2] == '2');
	CHECK(buffer[3] == '3');
	CHECK(buffer[4] == '4');
	CHECK(buffer[5] == '5');
	CHECK(buffer[6] == '6');
	CHECK(buffer[7] == '7');
}

TEST_CASE("StringUtil.HexadecimalToInteger")
{
	auto result0 = kokko::HexadecimalToInteger<uint32_t>("01234567");
	CHECK(result0.HasValue() == true);
	CHECK(result0.GetValue() == 0x1234567);

	auto result1 = kokko::HexadecimalToInteger<uint32_t>("abcdef01");
	CHECK(result1.HasValue() == true);
	CHECK(result1.GetValue() == 0xabcdef01);

	auto result2 = kokko::HexadecimalToInteger<uint32_t>("abcdEFAB");
	CHECK(result2.HasValue() == true);
	CHECK(result2.GetValue() == 0xabcdEFAB);

	auto result3 = kokko::HexadecimalToInteger<uint32_t>("0123456x");
	CHECK(result3.HasValue() == false);

	auto result4 = kokko::HexadecimalToInteger<uint32_t>("Qabcdef0");
	CHECK(result4.HasValue() == false);
}
