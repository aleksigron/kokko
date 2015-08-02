#include "Hash.h"

__attribute__((always_inline))
inline uint32_t rotl32(uint32_t x, int8_t r)
{
	return (x << r) | (x >> (32 - r));
}

__attribute__((always_inline))
inline uint32_t getblock32(const uint32_t* p, int i)
{
	return p[i];
}

__attribute__((always_inline))
inline uint32_t fmix32(uint32_t h)
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

	return h;
}

constexpr void Hash::Murmur3_32(const void* key, int length, uint32_t seed, void* out)
{
	const uint8_t * data = static_cast<const uint8_t*>(key);
	const int nblocks = length / 4;

	uint32_t h1 = seed;

	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;

	const uint32_t* blocks = reinterpret_cast<const uint32_t*>(data + nblocks * 4);

	for (int i = -nblocks; i; i++)
	{
		uint32_t k1 = getblock32(blocks,i);

		k1 *= c1;
		k1 = rotl32(k1,15);
		k1 *= c2;

		h1 ^= k1;
		h1 = rotl32(h1,13);
		h1 = h1*5+0xe6546b64;
	}

	const uint8_t * tail = data + nblocks * 4;

	uint32_t k1 = 0;

	switch(length & 3)
	{
		case 3:
			k1 ^= tail[2] << 16;
			[[clang::fallthrough]];
		case 2:
			k1 ^= tail[1] << 8;
			[[clang::fallthrough]];
		case 1:
			k1 ^= tail[0];
			k1 *= c1;
			k1 = rotl32(k1,15);
			k1 *= c2; h1 ^= k1;
	};

	h1 ^= length;

	h1 = fmix32(h1);

	*reinterpret_cast<uint32_t*>(out) = h1;
}
