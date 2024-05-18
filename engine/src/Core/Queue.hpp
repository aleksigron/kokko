#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <new>
#include <utility>

#include "Core/Core.hpp"

#include "Math/Math.hpp"

#include "Memory/Allocator.hpp"

namespace kokko
{

template <typename ValueType>
class Queue
{
private:
	Allocator* allocator;
	ValueType* data;
	size_t start;
	size_t count;
	size_t allocated;

	size_t GetArrayIndex(size_t queueIndex) const
	{
		assert(allocated != 0);
		assert(Math::IsPowerOfTwo(allocated));
		return (start + queueIndex) & (allocated - 1);
	}

	void ReserveInternal(size_t requiredSize)
	{
		if (requiredSize > allocated)
		{
			size_t newAllocated = allocated > 0 ? allocated * 2 : 8;
			if (newAllocated < requiredSize)
				newAllocated = Math::UpperPowerOfTwo(requiredSize);

			size_t newBytes = newAllocated * sizeof(ValueType);
			ValueType* newData = static_cast<ValueType*>(allocator->Allocate(newBytes, KOKKO_FUNC_SIG));

			size_t startToMemEnd = allocated - start;

			if (startToMemEnd < count) // Used data passes over the reserved memory end
			{
				// Copy memory from start of data to end of allocated memory
				std::memcpy(newData, data + start, startToMemEnd * sizeof(ValueType));

				// Copy memory from start of allocated memory to end of data
				std::memcpy(newData + startToMemEnd, data, (count - startToMemEnd) * sizeof(ValueType));
			}
			else // Entire used data is contiguous in reserved memory
			{
				std::memcpy(newData, data + start, count * sizeof(ValueType));
			}

			allocator->Deallocate(data);
			data = newData;
			start = 0;
			allocated = newAllocated;
		}
	}

public:
	Queue(Allocator* allocator) :
		allocator(allocator),
		data(nullptr),
		start(0),
		count(0),
		allocated(0)
	{
	}

	~Queue()
	{
		for (size_t i = 0; i < count; ++i)
			data[this->GetArrayIndex(i)].~ValueType();

		allocator->Deallocate(data);
	}

	size_t GetCount() const { return count; }

	ValueType& At(size_t index) { return data[this->GetArrayIndex(index)]; }
	const ValueType& At(size_t index) const { return data[this->GetArrayIndex(index)]; }

	ValueType& operator[](size_t index) { return data[this->GetArrayIndex(index)]; }
	const ValueType& operator[](size_t index) const { return data[this->GetArrayIndex(index)]; }

	ValueType& Peek() { return data[this->GetArrayIndex(0)]; }
	const ValueType& Peek() const { return data[this->GetArrayIndex(0)]; }

	ValueType Pop()
	{
		ValueType result;

		if (count > 0)
		{
			size_t frontIndex = this->GetArrayIndex(0);

			result = std::move(data[frontIndex]); // Move return value
			data[frontIndex].~ValueType(); // Run destructor

			start = this->GetArrayIndex(1);
			--count;
		}

		return result;
	}

	bool TryPop(ValueType& out)
	{
		if (count > 0)
		{
			size_t frontIndex = this->GetArrayIndex(0);

			out = std::move(data[frontIndex]); // Move return value
			data[frontIndex].~ValueType(); // Run destructor

			start = this->GetArrayIndex(1);
			--count;

			return true;
		}
		else
			return false;
	}

	void Pop(size_t popCount)
	{
		while (popCount > 0 && count > 0)
		{
			data[start].~ValueType(); // Run destructor
			start = this->GetArrayIndex(1);

			--popCount;
			--count;
		}
	}

	ValueType& Push()
	{
		this->ReserveInternal(count + 1);

		return *(new (data + this->GetArrayIndex(count++)) ValueType());
	}

	void Push(const ValueType& value)
	{
		ReserveInternal(count + 1);

		data[GetArrayIndex(count)] = value;
		++count;
	}

	void Push(const ValueType* values, size_t valueCount)
	{
		ReserveInternal(count + valueCount);

		for (size_t i = 0; i < valueCount; ++i)
		{
			data[GetArrayIndex(count)] = values[i];
			count += 1;
		}
	}

	void Clear()
	{
		for (size_t i = 0; i != count; ++i)
		{
			data[this->GetArrayIndex(i)].~ValueType(); // Run destructor
		}

		count = 0;
		start = 0;
	}
};

} // namespace kokko
