#pragma once

#include <cstdint>

#include "Core/ArrayView.hpp"
#include "Core/Optional.hpp"

namespace kokko
{

struct Uid
{
	uint64_t raw[2];

	static const size_t StringLength = sizeof(raw) * 2;

	Uid() : raw{ 0 }
	{
	}

	static Uid Create();
	static Optional<Uid> FromString(ArrayView<const char> str);

	bool operator==(const Uid& other) const
	{
		return raw[0] == other.raw[0] && raw[1] == other.raw[1];
	}

	bool operator!=(const Uid& other) const
	{
		return operator==(other) == false;
	}

	void WriteTo(ArrayView<char> out) const;
};

uint32_t HashValue32(const Uid& data, uint32_t seed);

}
