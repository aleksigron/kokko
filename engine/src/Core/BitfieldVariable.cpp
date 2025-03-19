#include "Core/BitfieldVariable.hpp"

#include <cstdint>

#include "doctest/doctest.h"

namespace kokko
{

TEST_CASE("BitfieldVariable.GetValue")
{
	BitfieldVariable<uint32_t> var0;
	var0.SetDefinition(10, sizeof(uint32_t) * 8);

	BitfieldVariable<uint32_t> var1;
	var1.SetDefinition(22, var0.shift);

	uint32_t val = 0;
	var0.AssignValue(val, 1023);
	var1.AssignValue(val, 11888);

	CHECK(var0.GetValue(val) == 1023);
	CHECK(var1.GetValue(val) == 11888);
}

TEST_CASE("BitfieldVariable.AssignZero")
{
	BitfieldVariable<uint32_t> var0;
	var0.SetDefinition(10, sizeof(uint32_t) * 8);

	BitfieldVariable<uint32_t> var1;
	var1.SetDefinition(22, var0.shift);

	uint32_t val = 0;
	var0.AssignValue(val, 1023);
	var1.AssignValue(val, 11888);

	var0.AssignZero(val);

	CHECK(var0.GetValue(val) == 0);
	CHECK(var1.GetValue(val) == 11888);

	var1.AssignZero(val);

	CHECK(var0.GetValue(val) == 0);
	CHECK(var1.GetValue(val) == 0);
}

} // namespace kokko
