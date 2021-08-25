#pragma once

#include <cstring>
#include <new>
#include <utility>

#include "Memory/Allocator.hpp"

template <typename ValueType>
class Queue
{
public:
	using SizeType = unsigned int;

private:
	Allocator* allocator;
	ValueType* data;
	SizeType start;
	SizeType count;
	SizeType allocated;

	SizeType GetArrayIndex(SizeType queueIndex) const
	{
		return (start + queueIndex) % allocated;
	}

	void ReserveInternal(SizeType requiredSize)
	{
		if (requiredSize > allocated)
		{
			std::size_t vts = sizeof(ValueType);
			SizeType newAllocated = allocated > 0 ? allocated * 2 : 8;
			std::size_t newSize = newAllocated * vts;
			ValueType* newData = static_cast<ValueType*>(allocator->Allocate(newSize));

			SizeType startToMemEnd = allocated - start;

			if (startToMemEnd < count) // Used data passes over the reserved memory end
			{
				// Copy memory from start of data to end of allocated memory
				std::memcpy(newData, data + start, startToMemEnd * vts);

				// Copy memory from start of allocated memory to end of data
				std::memcpy(newData + startToMemEnd, data, (count - startToMemEnd) * vts);
			}
			else // Entire used data is contiguous in reserved memory
			{
				std::memcpy(newData, data + start, count * vts);
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
		for (SizeType i = 0; i < count; ++i)
			data[this->GetArrayIndex(i)].~ValueType();

		allocator->Deallocate(data);
	}

	SizeType GetCount() const { return count; }

	ValueType& At(SizeType index) { return data[this->GetArrayIndex(index)]; }
	const ValueType& At(SizeType index) const { return data[this->GetArrayIndex(index)]; }

	ValueType& operator[](SizeType index) { return data[this->GetArrayIndex(index)]; }
	const ValueType& operator[](SizeType index) const { return data[this->GetArrayIndex(index)]; }

	ValueType& Peek() { return data[this->GetArrayIndex(0)]; }
	const ValueType& Peek() const { return data[this->GetArrayIndex(0)]; }

	ValueType Pop()
	{
		ValueType result;

		if (count > 0)
		{
			SizeType frontIndex = this->GetArrayIndex(0);

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
			SizeType frontIndex = this->GetArrayIndex(0);

			out = std::move(data[frontIndex]); // Move return value
			data[frontIndex].~ValueType(); // Run destructor

			start = this->GetArrayIndex(1);
			--count;

			return true;
		}
		else
			return false;
	}

	void Pop(SizeType popCount)
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

	void Push(const ValueType* values, SizeType valueCount)
	{
		ReserveInternal(count + valueCount);

		for (SizeType i = 0; i < valueCount; ++i)
		{
			data[GetArrayIndex(count++)] = values[i];
		}
	}
};
