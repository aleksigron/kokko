#pragma once

#include <cstdint>

#include "Core/Optional.hpp"
#include "Core/ArrayView.hpp"

struct StringRef;

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

	operator bool()
	{
		return raw[0] != 0 || raw[1] != 0;
	}

	void WriteTo(ArrayView<char> out) const;
};

uint32_t Hash32(const Uid& uid, uint32_t seed);

}
