#pragma once

#include <cstring>
#include <new>

template <typename ValueType>
class Queue
{
public:
	using SizeType = unsigned int;

private:
	ValueType* data;
	SizeType start;
	SizeType count;
	SizeType allocated;

	SizeType GetArrayIndex(SizeType queueIndex) const
	{
		return (start + queueIndex) % allocated;
	}

	void Reallocate(SizeType requiredSize)
	{
		if (requiredSize > allocated)
		{
			SizeType newAllocated = allocated > 0 ? allocated * 2 : 8;
			ValueType* newData = static_cast<ValueType*>(operator new[](newAllocated * sizeof(ValueType)));

			SizeType startToMemEnd = allocated - start;
			std::size_t vts = sizeof(ValueType);

			if (startToMemEnd < count)
			{
				// Copy memory from start to end of allocated memory
				std::memcpy(newData, data + start, startToMemEnd * vts);

				// Copy memory from start of allocated memory
				std::memcpy(newData + startToMemEnd, data, (count - startToMemEnd) * vts);
			}
			else
			{
				// Copy entire block of data
				std::memcpy(newData, data + start, count * vts);
			}

			operator delete[](data);
			data = newData;
			start = 0;
			allocated = newAllocated;
		}
	}

public:
	Queue() :
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

		operator delete(data);
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
			result = data[this->GetArrayIndex(0)];
			start = this->GetArrayIndex(1);
			--count;
		}
		else
			result = ValueType();

		return result;
	}

	ValueType& Push()
	{
		this->Reallocate(count + 1);

		return *(new (data + this->GetArrayIndex(count++)) ValueType());
	}

	void Push(const ValueType& value)
	{
		this->Reallocate(count + 1);

		data[this->GetArrayIndex(count)] = value;
		++count;
	}
};
