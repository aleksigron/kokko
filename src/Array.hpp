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

	// Get a reference to the last item in the array
	ValueType& GetBack() { return this->data[this->count - 1]; }

	// Get a reference to the last item in the array
	const ValueType& GetBack() const { return this->data[this->count - 1]; }

	// Get a reference to the specified index in the array
	ValueType& At(SizeType index) { return this->data[index]; }

	// Get a reference to the specified index in the array
	const ValueType& At(SizeType index) const { return this->data[index]; }

	ValueType& operator[](SizeType index) { return this->data[index]; }
	const ValueType& operator[](SizeType index) const { return this->data[index]; }

	// Make sure there's at least the specified amount of space in the array
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

	// Add a new item to the back of the array and return a reference to the item
	ValueType& PushBack()
	{
		this->ReserveInternal(this->count + 1);
		ValueType* value = new (this->data + this->count) ValueType;
		++(this->count);
		return *value;
	}

	// Add the passed item to the back of the array
	void PushBack(const ValueType& value)
	{
		this->ReserveInternal(this->count + 1);
		this->data[this->count] = value;
		++(this->count);
	}

	// Copy the specified items to the back of the array
	void InsertBack(const ValueType* items, SizeType count)
	{
		this->ReserveInternal(this->count + count);

		for (SizeType i = 0; i < count; ++i)
		{
			this->data[this->count] = items[i];
			++(this->count);
		}
	}

	// Remove the last item in the array
	void PopBack()
	{
		--(this->count);

		this->data[this->count].~ValueType();
	}

	// Remove all items from the array
	void Clear()
	{
		for (SizeType i = 0; i < this->count; ++i)
		{
			this->data[i].~ValueType();
		}

		this->count = 0;
	}
};
