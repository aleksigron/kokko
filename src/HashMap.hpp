#pragma once

#include <cstring>

#include "Hash.hpp"
#include "Math.hpp"

// Based on https://github.com/preshing/CompareIntegerMaps

template <typename KeyType, typename ValueType>
class HashMap
{
public:
	struct KeyValuePair
	{
		KeyType key;
		ValueType value;
	};

private:
	KeyValuePair* data;
	unsigned int population;
	unsigned int allocated;

	bool zeroUsed;
	KeyValuePair zeroPair;

	unsigned int GetIndex(unsigned int hash) const { return hash & (allocated - 1); }
	unsigned int GetOffset(KeyValuePair* a, KeyValuePair* b) const
	{
		return b >= a ? b - a : allocated + b - a;
	}

public:
	HashMap() :
		data(nullptr),
		population(0),
		allocated(0),
		zeroUsed(false)
	{
		zeroPair = KeyValuePair{};
	}

	~HashMap()
	{
		if (data != nullptr)
		{
			KeyValuePair* itr = data;
			KeyValuePair* end = data + allocated;
			for (; itr != end; ++itr)
				if (itr->key)
					itr->value.~ValueType();

			delete[] data;
		}
	}

	void ReserveInternal(unsigned int desiredSize)
	{
		KeyValuePair* newData = new KeyValuePair[desiredSize];
		std::memset(newData, 0, sizeof(KeyValuePair) * desiredSize);

		if (data != nullptr) // Old data exists
		{
			KeyValuePair* p = data;
			KeyValuePair* end = p + allocated;
			for (; p != end; ++p)
			{
				if (p->key) // Pair has value
				{
					for (unsigned int i = GetIndex(Hash::FNV1a_32(p->key));; i = GetIndex(i + 1))
					{
						if (!data[i].key) // Insert here
						{
							data[i] = *p;
							break;
						}
					}
				}
			}

			delete[] data;
		}

		data = newData;
		allocated = desiredSize;
	}

	KeyValuePair* Lookup(KeyType key)
	{
		if (key)
		{
			if (data != nullptr)
			{
				for (unsigned int i = GetIndex(Hash::FNV1a_32(key));; i = GetIndex(i + 1))
				{
					KeyValuePair* pair = data + i;

					if (pair->key == key)
						return pair;

					if (!pair->key)
						return nullptr;
				}
			}
		}
		else if (zeroUsed)
			return &zeroPair;

		return nullptr;
	}

	KeyValuePair* Insert(KeyType key)
	{
		if (key)
		{
			if (data == nullptr)
				ReserveInternal(16);

			for (;;)
			for (unsigned int i = GetIndex(Hash::FNV1a_32(key));; i = GetIndex(i + 1))
			{
				KeyValuePair* pair = data + i;

				if (pair->key == key) // Found
					return pair;

				if (pair->key == 0) // Insert here
				{
					if ((population + 1) * 4 >= allocated * 3)
					{
						ReserveInternal(allocated * 2);
						break; // Back to outer loop, find key again
					}

					++population;
					pair->key = key;
					return pair;
				}
			}
		}
		else
		{
			if (zeroUsed == false)
			{
				zeroUsed = true;
				++population;

				// Even though we didn't use a regular slot, let's keep the sizing rules consistent
				if (population * 4 >= allocated * 3)
					ReserveInternal(allocated * 2);
			}

			return &zeroPair;
		}
	}

	void Remove(KeyValuePair* pair)
	{
		if (pair != &zeroPair)
		{
			if (data != nullptr &&
				pair >= data && pair < data + allocated &&
				pair->key)
			{
				// Remove this cell by shuffling neighboring cells
				// so there are no gaps in anyone's probe chain
				for (unsigned int i = GetIndex(pair - data + 1);; i = GetIndex(i + 1))
				{
					KeyValuePair* neighbor = data + i;

					if (!neighbor->key)
					{
						// There's nobody to swap with. Go ahead and clear this cell, then return
						pair->key = KeyType{};
						pair->value = ValueType{};
						population--;
						return;
					}

					KeyValuePair* ideal = GetIndex(Hash::FNV1a_32(neighbor->key));
					if (GetOffset(ideal, pair) < GetOffset(ideal, neighbor))
					{
						// Swap with neighbor, then make neighbor the new cell to remove.
						*pair = *neighbor;
						pair = neighbor;
					}
				}
			}
		}
		else if (zeroUsed) // Ignore if not in use
		{
			zeroUsed = false;
			pair->value = 0;
			population--;
		}
	}

	void Reserve(unsigned int desiredPopulation)
	{
		if (desiredPopulation < population)
			return;

		unsigned int desiredSize = (desiredPopulation * 4 + 2) / 3;

		// Desired size is not a power-of-two
		if ((desiredSize & (desiredSize - 1)) != 0)
			desiredSize = Math::UpperPowerOfTwo(desiredSize);

		this->ReserveInternal(desiredSize);
	}
};
