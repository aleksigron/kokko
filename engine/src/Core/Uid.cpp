#include "Core/Uid.hpp"

#include <limits>

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

Optional<Uid> Uid::FromString(StringRef str)
{
	Optional<Uid> result;

	if (str.len >= 32)
	{
		auto result0 = HexadecimalToInteger<uint64_t>(&str.str[0]);
		auto result1 = HexadecimalToInteger<uint64_t>(&str.str[sizeof(raw[0]) * 2]);

		if (result0.HasValue() && result1.HasValue())
		{
			Uid uid;
			uid.raw[0] = result0;
			uid.raw[1] = result1;
			result = uid;
		}
	}

	return result;
}

void Uid::WriteToBuffer(char* out)
{
	IntegerToHexadecimal(raw[0], &out[0]);
	IntegerToHexadecimal(raw[1], &out[sizeof(raw[0]) * 2]);
}

uint32_t Hash32(const Uid& uid, uint32_t seed)
{
	return Hash32(&uid, sizeof(Uid), seed);
}

}
