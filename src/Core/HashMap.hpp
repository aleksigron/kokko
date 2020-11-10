#pragma once

#include <cstring>

#include "Core/Pair.hpp"
#include "Core/Hash.hpp"
#include "Math/Math.hpp"
#include "Memory/Allocator.hpp"

// Based on https://github.com/preshing/CompareIntegerMaps

template <typename KeyType, typename ValueType>
class HashMap
{
public:
	using KeyValuePair = Pair<KeyType, ValueType>;

	class Iterator
	{
	private:
		KeyValuePair* current;
		KeyValuePair* end; // We need end to know when to stop in operator++()

		friend class HashMap;

	public:
		KeyValuePair& operator*() { return *current; }
		KeyValuePair* operator->() { return current; }

		bool operator==(Iterator other) const { return this->current == other.current; }
		bool operator!=(Iterator other) const { return operator==(other) == false; }

		Iterator& operator++()
		{
			if (current == end)
				return *this;

			while (true)
			{
				++current;

				if (current == end || current->first)
				{
					break;
				}
			}

			return *this;
		}

		Iterator operator++(int)
		{
			Iterator itr = *this;
			operator++();
			return itr;
		}
	};

private:
	Allocator* allocator;
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

	void ReserveInternal(unsigned int desiredCount)
	{
		std::size_t newSize = desiredCount * sizeof(KeyValuePair);
		KeyValuePair* newData = static_cast<KeyValuePair*>(allocator->Allocate(newSize));
		std::memset(newData, 0, newSize);

		if (data != nullptr) // Old data exists
		{
			KeyValuePair* p = data;
			KeyValuePair* end = p + allocated;
			for (; p != end; ++p)
			{
				if (p->first) // Pair has value
				{
					for (unsigned int i = GetIndex(Hash::FNV1a_32(p->first));; i = GetIndex(i + 1))
					{
						if (!newData[i].first) // Insert here
						{
							newData[i] = *p;
							break;
						}
					}
				}
			}

			allocator->Deallocate(data);
		}

		data = newData;
		allocated = desiredCount;
	}

public:
	HashMap(Allocator* allocator) :
		allocator(allocator),
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
				if (itr->first)
					itr->second.~ValueType();

			allocator->Deallocate(data);
		}
	}

	Iterator Begin()
	{
		Iterator itr;
		itr.end = this->data + this->allocated;

		if (this->data != nullptr)
		{
			// Start at one before this->data
			itr.current = this->data - 1;

			// Find first valid item with the real increment operator
			++itr;
		}
		else
			itr.current = itr.end;

		return itr;
	}

	Iterator End()
	{
		Iterator itr;
		itr.end = this->data + this->allocated;
		itr.current = itr.end;
		return itr;
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

					if (pair->first == key)
						return pair;

					if (!pair->first)
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

				if (pair->first == key) // Found
					return pair;

				if (pair->first == 0) // Insert here
				{
					if ((population + 1) * 4 >= allocated * 3)
					{
						ReserveInternal(allocated * 2);
						break; // Back to outer loop, find first again
					}

					++population;
					pair->first = key;
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
				pair->first)
			{
				// Remove this cell by shuffling neighboring cells
				// so there are no gaps in anyone's probe chain
				for (unsigned int i = GetIndex(pair - data + 1);; i = GetIndex(i + 1))
				{
					KeyValuePair* neighbor = data + i;

					if (!neighbor->first)
					{
						// There's nobody to swap with. Go ahead and clear this cell, then return
						pair->first = KeyType{};
						pair->second = ValueType{};
						population--;
						return;
					}

					KeyValuePair* ideal = GetIndex(Hash::FNV1a_32(neighbor->first));
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
			pair->second = ValueType{};
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
