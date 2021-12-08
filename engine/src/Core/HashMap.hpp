#pragma once

#include "Core/Core.hpp"
#include "Core/Pair.hpp"
#include "Core/Hash.hpp"

#include "Math/Math.hpp"

#include "Memory/Allocator.hpp"

// Based on https://github.com/preshing/CompareIntegerMaps
// TODO: Make sure iterator will also go through zero pair

template <typename KeyType, typename ValueType>
class HashMap
{
public:
	using KeyValuePair = Pair<KeyType, ValueType>;

	class Iterator
	{
	private:
		KeyValuePair* current;
		KeyValuePair* zero;
		KeyValuePair* begin;
		KeyValuePair* end;

		friend class HashMap;

	public:
		KeyValuePair& operator*() { return *current; }
		KeyValuePair* operator->() { return current; }

		bool operator==(const Iterator& other) const { return this->current == other.current; }
		bool operator!=(const Iterator& other) const { return operator==(other) == false; }

		Iterator& operator++()
		{
			if (current == nullptr || current == end)
			{
				return *this;
			}
			else if (current == zero)
			{
				current = begin - 1;
			}

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
	size_t population;
	size_t allocated;

	bool zeroUsed;
	KeyValuePair zeroPair;

	size_t GetIndex(size_t hash) const { return hash & (allocated - 1); }
	size_t GetOffset(KeyValuePair* a, KeyValuePair* b) const
	{
		return b >= a ? b - a : allocated + b - a;
	}

	void ReserveInternal(size_t desiredCount)
	{
		size_t newSize = desiredCount * sizeof(KeyValuePair);
		KeyValuePair* newData = static_cast<KeyValuePair*>(allocator->Allocate(newSize, KOKKO_FUNC_SIG));
		std::memset(newData, 0, newSize);

		if (data != nullptr) // Old data exists
		{
			KeyValuePair* p = data;
			KeyValuePair* end = p + allocated;

			// this->allocated needs to be set here because GetIndex() uses it
			allocated = desiredCount;

			for (; p != end; ++p)
			{
				if (p->first != KeyType{}) // Pair has value
				{
					for (size_t i = GetIndex(kokko::Hash32(p->first, 0));; i = GetIndex(i + 1))
					{
						if (newData[i].first == KeyType{}) // Insert here
						{
							newData[i] = *p;
							break;
						}
					}
				}
			}

			allocator->Deallocate(data);
		}
		else
			allocated = desiredCount;

		data = newData;
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
				if (itr->first != KeyType{})
					itr->second.~ValueType();

			if (zeroUsed)
				zeroPair.second.~ValueType();

			allocator->Deallocate(data);
		}
	}

	void Clear()
	{
		KeyValuePair* itr = data;
		KeyValuePair* end = data + allocated;
		for (; itr != end; ++itr)
		{
			if (itr->first != KeyType{})
			{
				itr->second.~ValueType();
				itr->first = KeyType{};
			}
		}

		population = 0;

		if (zeroUsed)
		{
			zeroPair.second.~ValueType();
			zeroUsed = false;
		}
	}

	Iterator begin()
	{
		Iterator itr;

		if (data != nullptr)
		{
			// Start at one before this->data
			itr.begin = data;
			itr.end = itr.begin + allocated;

			if (zeroUsed)
			{
				itr.current = &zeroPair;
				itr.zero = &zeroPair;
			}
			else
			{
				itr.current = data - 1;
				itr.zero = nullptr;

				// Find first valid item with the real increment operator
				++itr;
			}
		}
		else
		{
			itr.current = nullptr;
			itr.zero = nullptr;
			itr.begin = nullptr;
			itr.end = nullptr;
		}

		return itr;
	}

	Iterator end()
	{
		Iterator itr;
		itr.begin = data;
		itr.end = itr.begin + allocated;
		itr.current = itr.end;
		itr.zero = zeroUsed ? &zeroPair : nullptr;

		return itr;
	}

	KeyValuePair* Lookup(KeyType key)
	{
		if (key)
		{
			if (data != nullptr)
			{
				for (size_t i = GetIndex(kokko::Hash32(key, 0));; i = GetIndex(i + 1))
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
		if (data == nullptr)
			ReserveInternal(16);

		if (key != KeyType{})
		{
			for (;;)
			for (size_t i = GetIndex(kokko::Hash32(key, 0));; i = GetIndex(i + 1))
			{
				KeyValuePair* pair = data + i;

				if (pair->first == key) // Found
					return pair;

				if (pair->first == KeyType{}) // Insert here
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
				for (size_t i = GetIndex(pair - data + 1);; i = GetIndex(i + 1))
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

					KeyValuePair* ideal = data + GetIndex(kokko::Hash32(neighbor->first, 0));
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

	void Reserve(size_t desiredPopulation)
	{
		if (desiredPopulation < population)
			return;

		size_t desiredSize = (desiredPopulation * 4 + 2) / 3;

		// Desired size is not a power-of-two
		if ((desiredSize & (desiredSize - 1)) != 0)
			desiredSize = Math::UpperPowerOfTwo(desiredSize);

		this->ReserveInternal(desiredSize);
	}
};
