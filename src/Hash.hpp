#pragma once

#include <cstdint>
#include <cstddef>

namespace Hash
{
	constexpr uint32_t FNV1a_32Basis = 0x811c9dc5;
	constexpr uint32_t FNV_32MagicPrime = 0x01000193;

	constexpr uint32_t FNV1a_32(const unsigned char* data, size_t length)
	{
		uint32_t hash = FNV1a_32Basis;

		for (const unsigned char* end = data + length; data < end; ++data)
		{
			hash *= FNV_32MagicPrime;
			hash ^= static_cast<uint32_t>(*data);
		}

		return hash;
	}


	constexpr uint32_t FNV1a_32(const char* string, size_t length)
	{
		uint32_t hash = FNV1a_32Basis;

		for (const char* end = string + length; string < end; ++string)
		{
			hash *= FNV_32MagicPrime;
			hash ^= static_cast<uint32_t>(*string);
		}

		return hash;
	}

	constexpr uint32_t FNV1a_32(uint32_t v)
	{
		return (FNV1a_32Basis * FNV_32MagicPrime) ^ v;
	}

	constexpr uint32_t FNV1a_32(int32_t v)
	{
		return (FNV1a_32Basis * FNV_32MagicPrime) ^ static_cast<uint32_t>(v);
	}
}

constexpr uint32_t operator ""_hash(const char* string, size_t size)
{
	return Hash::FNV1a_32(string, size);
}
