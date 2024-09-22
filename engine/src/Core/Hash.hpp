#pragma once

#include <cstdint>
#include <cstddef>

namespace kokko
{

template<typename T>
struct Hash32 {
	uint32_t operator()(const T& val, uint8_t seed) const { return HashValue32(val, seed); }
};

uint64_t HashValue64(const void* data, size_t length, uint64_t seed);

uint32_t HashValue32(const void* data, size_t length, uint32_t seed);

uint32_t HashValue32(uint8_t data, uint32_t seed);
uint32_t HashValue32(int8_t data, uint32_t seed);
uint32_t HashValue32(uint16_t data, uint32_t seed);
uint32_t HashValue32(int16_t data, uint32_t seed);
uint32_t HashValue32(uint32_t data, uint32_t seed);
uint32_t HashValue32(int32_t data, uint32_t seed);
uint32_t HashValue32(uint64_t data, uint32_t seed);
uint32_t HashValue32(int64_t data, uint32_t seed);

// TODO: Figure out a way to declare the overloads in the types' own header file
// Currently GCC would fail compilation because it doesn't consider those declarations
// to be overloads of the original hash functions.

constexpr uint32_t HashString(const char* string, size_t length)
{
	constexpr uint32_t FNV1a_32Basis = 0x811c9dc5;
	constexpr uint32_t FNV_32MagicPrime = 0x01000193;

	uint32_t hash = FNV1a_32Basis;

	for (const char* end = string + length; string < end; ++string)
	{
		hash *= FNV_32MagicPrime;
		hash ^= static_cast<uint32_t>(*string);
	}

	return hash;
}

}

constexpr uint32_t operator ""_hash(const char* string, size_t size)
{
	return kokko::HashString(string, size);
}
