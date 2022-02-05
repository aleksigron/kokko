#include "Core/Uid.hpp"

#include <cassert>
#include <limits>

#include "doctest/doctest.h"

#include "Core/Hash.hpp"
#include "Core/StringRef.hpp"
#include "Core/StringUtil.hpp"

#include "Math/Random.hpp"

namespace kokko
{

Uid Uid::Create()
{
	Uid result;

	result.raw[0] = Random::Uint64(0, std::numeric_limits<uint64_t>::max());
	result.raw[1] = Random::Uint64(0, std::numeric_limits<uint64_t>::max());

	return result;
}

Optional<Uid> Uid::FromString(ArrayView<const char> str)
{
	Optional<Uid> result;

	if (str.GetCount() >= StringLength)
	{
		constexpr size_t charsPerRaw = sizeof(raw[0]) * 2;

		auto result0 = HexadecimalToInteger<uint64_t>(str.GetSubView(0, charsPerRaw));
		auto result1 = HexadecimalToInteger<uint64_t>(str.GetSubView(charsPerRaw, charsPerRaw * 2));

		if (result0.HasValue() && result1.HasValue())
		{
			Uid uid;
			uid.raw[0] = result0.GetValue();
			uid.raw[1] = result1.GetValue();
			result = uid;
		}
	}

	return result;
}

void Uid::WriteTo(ArrayView<char> out) const
{
	assert(out.GetCount() >= StringLength);
	constexpr size_t charsPerRaw = sizeof(raw[0]) * 2;
	IntegerToHexadecimal(raw[0], out.GetSubView(0, charsPerRaw));
	IntegerToHexadecimal(raw[1], out.GetSubView(charsPerRaw, charsPerRaw * 2));
}

uint32_t Hash32(const Uid& uid, uint32_t seed)
{
	return Hash32(&uid, sizeof(Uid), seed);
}

TEST_CASE("Uid.Serialization")
{
	// Check serialization and deserialization with 20 random generated values
	for (int i = 0; i < 20; ++i)
	{
		char buffer[Uid::StringLength];
		Uid original = Uid::Create();
		original.WriteTo(buffer);
		auto result = Uid::FromString(ArrayView<const char>(buffer, sizeof(buffer)));
		CHECK(result.HasValue() == true);
		CHECK(result.GetValue() == original);
	}

	const char validString[] = "0123456789abcdef0123456789abcdef";
	auto result0 = Uid::FromString(ArrayView(validString, sizeof(validString) - 2));
	CHECK(result0.HasValue() == false);

	auto result1 = Uid::FromString(ArrayView(validString, 0));
	CHECK(result1.HasValue() == false);

	const char invalidString[] = "0123456789abcdef-123456789abcdef";
	auto result2 = Uid::FromString(ArrayView(invalidString));
	CHECK(result2.HasValue() == false);
}

}
