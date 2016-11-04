#pragma once

#include <cstring>
#include <new>

template <typename ValueType>
class Array
{
public:
	using SizeType = unsigned int;

private:
	ValueType* data;
	SizeType count;
	SizeType allocated;

	void ReserveInternal(SizeType required)
	{
		if (required > this->allocated)
		{
			SizeType newAllocated = this->allocated > 1 ? this->allocated * 2 : 4;

			if (required > newAllocated)
				newAllocated = required;

			ValueType* newData = static_cast<ValueType*>(operator new[](newAllocated * sizeof(ValueType)));

			// There is old data
			if (this->count > 0)
			{
				// Copy old data to new buffer
				std::memcpy(newData, this->data, this->count * sizeof(ValueType));

				// Delete old buffer
				operator delete[](this->data);
			}

			this->data = newData;
			this->allocated = newAllocated;
		}
	}

public:
	Array() : data(nullptr), count(0), allocated(0)
	{
	}

	~Array()
	{
		delete[] data;
	}

	SizeType GetCount() const { return this->count; }

	ValueType* GetData() { return this->data; }
	const ValueType* GetData() const { return this->data; }

	ValueType& GetBack() { return this->data[this->count - 1]; }
	const ValueType& GetBack() const { return this->data[this->count - 1]; }

	ValueType& At(SizeType index) { return this->data[index]; }
	const ValueType& At(SizeType index) const { return this->data[index]; }

	ValueType& operator[](SizeType index) { return this->data[index]; }
	const ValueType& operator[](SizeType index) const { return this->data[index]; }

	void Reserve(SizeType required)
	{
		if (required > this->allocated)
		{
			ValueType* newData = static_cast<ValueType*>(operator new[](required * sizeof(ValueType)));

			// There is old data
			if (this->count > 0)
			{
				// Copy old data to new buffer
				std::memcpy(newData, this->data, this->count * sizeof(ValueType));

				// Delete old buffer
				operator delete[](this->data);
			}

			this->data = newData;
			this->allocated = required;
		}
	}

	ValueType& PushBack()
	{
		this->ReserveInternal(this->count + 1);
		ValueType* value = new (this->data + this->count) ValueType;
		++(this->count);
		return *value;
	}

	void PushBack(const ValueType& value)
	{
		this->ReserveInternal(this->count + 1);
		this->data[this->count] = value;
		++(this->count);
	}

	void InsertBack(const ValueType* items, SizeType count)
	{
		this->ReserveInternal(this->count + count);

		for (SizeType i = 0; i < count; ++i)
		{
			this->data[this->count] = items[i];
			++(this->count);
		}
	}

	void PopBack()
	{
		--(this->count);

		this->data[this->count].~ValueType();
	}

	void Clear()
	{
		for (SizeType i = 0; i < this->count; ++i)
		{
			this->data[i].~ValueType();
		}

		this->count = 0;
	}
};
