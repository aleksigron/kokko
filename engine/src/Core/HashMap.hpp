#pragma once

#include <new>

#include "Core/Core.hpp"
#include "Core/Hash.hpp"
#include "Core/Pair.hpp"
#include "Core/TypeTraits.hpp"

#include "Math/Math.hpp"

#include "Memory/Allocator.hpp"

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

				if (current == end || !(current->first == KeyType{}))
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

		if constexpr (kokko::IsZeroInitializable<KeyType>::Value)
			std::memset(newData, 0, newSize);
		else
			for (size_t i = 0; i < desiredCount; ++i)
				new (&newData[i].first) KeyType;

		if (data != nullptr) // Old data exists
		{
			KeyValuePair* existing = data;
			KeyValuePair* end = existing + allocated;

			// This needs to be set here because GetIndex() uses it
			allocated = desiredCount;

			for (; existing != end; ++existing)
			{
				if (!(existing->first == KeyType{})) // Pair has value
				{
					for (size_t i = GetIndex(kokko::Hash32(existing->first, 0u));; i = GetIndex(i + 1))
					{
						if (newData[i].first == KeyType{}) // Insert here
						{
							newData[i].first = std::move(existing->first);
							new (&newData[i].second) ValueType(std::move(existing->second));

							if constexpr (std::is_trivially_destructible<KeyType>::value == false)
								existing->first.~KeyType();
							if constexpr (std::is_trivially_destructible<ValueType>::value == false)
								existing->second.~ValueType();

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
	explicit HashMap(Allocator* allocator) :
		allocator(allocator),
		data(nullptr),
		population(0),
		allocated(0),
		zeroUsed(false),
		zeroPair{}
	{
	}

	HashMap(const HashMap& other) :
		allocator(other.allocator),
		data(nullptr),
		population(other.population),
		allocated(other.allocated),
		zeroUsed(other.zeroUsed),
		zeroPair(other.zeroPair)
	{
		if (other.data != nullptr && other.population > 0)
		{
			ReserveInternal(other.allocated);
			std::memcpy(data, other.data, other.allocated * sizeof(KeyValuePair));
		}
	}

	HashMap(HashMap&& other) noexcept :
		allocator(other.allocator),
		data(other.data),
		population(other.population),
		allocated(other.allocated),
		zeroUsed(other.zeroUsed),
		zeroPair(other.zeroPair)
	{
		other.data = nullptr;
		other.population = 0;
		other.allocated = 0;
		other.zeroUsed = false;
		other.zeroPair = KeyValuePair{};
	}

	~HashMap()
	{
		if (data != nullptr)
		{
			KeyValuePair* itr = data;
			KeyValuePair* end = data + allocated;
			for (; itr != end; ++itr)
				if (!(itr->first == KeyType{}))
					itr->second.~ValueType();

			if (zeroUsed)
				zeroPair.second.~ValueType();

			allocator->Deallocate(data);
		}
	}

	HashMap& operator=(const HashMap& other)
	{
		if (data != nullptr)
		{
			allocator->Deallocate(data);
			data = nullptr;
		}

		allocator = other.allocator;
		population = other.population;
		allocated = other.allocated;
		zeroUsed = other.zeroUsed;
		zeroPair = other.zeroPair;

		if (other.data != nullptr && other.population > 0)
		{
			ReserveInternal(other.allocated);
			std::memcpy(data, other.data, other.allocated * sizeof(KeyValuePair));
		}

		return *this;
	}

	HashMap& operator=(HashMap&& other)
	{
		if (data != nullptr)
		{
			allocator->Deallocate(data);
		}

		allocator = other.allocator;
		data = other.data;
		population = other.population;
		allocated = other.allocated;
		zeroUsed = other.zeroUsed;
		zeroPair = other.zeroPair;

		other.data = nullptr;
		other.population = 0;
		other.allocated = 0;
		other.zeroUsed = false;
		other.zeroPair = KeyValuePair{};

		return *this;
	}

	void Clear()
	{
		KeyValuePair* itr = data;
		KeyValuePair* end = data + allocated;
		for (; itr != end; ++itr)
		{
			if (!(itr->first == KeyType{}))
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

	KeyValuePair* Lookup(const KeyType& key)
	{
		if (!(key == KeyType{}))
		{
			if (data != nullptr)
			{
				for (size_t i = GetIndex(kokko::Hash32(key, 0u));; i = GetIndex(i + 1))
				{
					KeyValuePair* pair = data + i;

					if (pair->first == key)
						return pair;

					if (pair->first == KeyType{})
						return nullptr;
				}
			}
		}
		else if (zeroUsed)
			return &zeroPair;

		return nullptr;
	}

	const KeyValuePair* Lookup(const KeyType& key) const
	{
		if (key != KeyType{})
		{
			if (data != nullptr)
			{
				for (size_t i = GetIndex(kokko::Hash32(key, 0u));; i = GetIndex(i + 1))
				{
					KeyValuePair* pair = data + i;

					if (pair->first == key)
						return pair;

					if (pair->first == KeyType{})
						return nullptr;
				}
			}
		}
		else if (zeroUsed)
			return &zeroPair;

		return nullptr;
	}

	KeyValuePair* Insert(const KeyType& key)
	{
		if (data == nullptr)
			ReserveInternal(16);

		if (!(key == KeyType{}))
		{
			for (;;)
			for (size_t i = GetIndex(kokko::Hash32(key, 0u));; i = GetIndex(i + 1))
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
					new (&pair->second) ValueType;

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
				pair >= data &&
				pair < data + allocated &&
				pair->first != KeyType{})
			{
				// Remove this cell by shuffling neighboring cells
				// so there are no gaps in anyone's probe chain
				for (size_t i = GetIndex(pair - data + 1);; i = GetIndex(i + 1))
				{
					KeyValuePair* neighbor = data + i;

					if (neighbor->first == KeyType{})
					{
						// There's nobody to swap with. Go ahead and clear this cell, then return
						pair->first = KeyType{};
						pair->second = ValueType{};
						population--;
						return;
					}

					KeyValuePair* ideal = data + GetIndex(kokko::Hash32(neighbor->first, 0u));
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
