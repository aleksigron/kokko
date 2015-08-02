#pragma once

#include <cstdint>
#include <cstddef>

namespace Hash
{
	constexpr uint32_t FNV1a_32(const char *string, size_t length)
	{
		// FNV1a-32 basis
		uint32_t hash = 0x811c9dc5;

		for (size_t i = 0; i < length; ++i)
		{
			// FNV-32 magic prime
			hash *= 0x01000193;
			hash ^= static_cast<uint32_t>(*(string + i));
		}

		return hash;
	}
}

constexpr uint32_t operator ""_hash(const char* string, size_t size)
{
	return Hash::FNV1a_32(string, size);
}