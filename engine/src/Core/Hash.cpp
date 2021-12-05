#include "Core/Hash.hpp"

#include "xxhash.h"

namespace kokko
{
uint32_t Hash32(const void* data, size_t length, uint32_t seed)
{
	return XXH32(data, length, seed);
}

uint32_t Hash32(uint8_t data, uint32_t seed)
{
	return Hash32(&data, sizeof(data), seed);
}

uint32_t Hash32(int8_t data, uint32_t seed)
{
	return Hash32(&data, sizeof(data), seed);
}

uint32_t Hash32(uint16_t data, uint32_t seed)
{
	return Hash32(&data, sizeof(data), seed);
}

uint32_t Hash32(int16_t data, uint32_t seed)
{
	return Hash32(&data, sizeof(data), seed);
}

uint32_t Hash32(uint32_t data, uint32_t seed)
{
	return Hash32(&data, sizeof(data), seed);
}

uint32_t Hash32(int32_t data, uint32_t seed)
{
	return Hash32(&data, sizeof(data), seed);
}

uint32_t Hash32(uint64_t data, uint32_t seed)
{
	return Hash32(&data, sizeof(data), seed);
}

uint32_t Hash32(int64_t data, uint32_t seed)
{
	return Hash32(&data, sizeof(data), seed);
}

uint64_t Hash64(const void* data, size_t length, uint64_t seed)
{
	return XXH64(data, length, seed);
}
}
