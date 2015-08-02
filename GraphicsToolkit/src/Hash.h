#pragma once

#include <cstdint>
#include <cstddef>

namespace Hash
{
	constexpr void Murmur3_32(const void* key, int length, uint32_t seed, void* out);
}

constexpr uint32_t operator ""_hash (const char * string, size_t size)
{
	uint32_t hash = 0;
	Hash::Murmur3_32(string, static_cast<int>(size), 0, &hash);
	return hash;
}