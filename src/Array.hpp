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

			const std::size_t vts = sizeof(ValueType);

			ValueType* newData = static_cast<ValueType*>(operator new[](newAllocated * vts));

			// There is old data
			if (this->count > 0)
			{
				// Copy old data to new buffer
				std::memcpy(newData, this->data, this->count * vts);

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

	/**
	 * Make sure there's at least the specified amount of space in the array
	 */
	void Reserve(SizeType required)
	{
		this->ReserveInternal(required);
	}

	/**
	 * Add a new item to the back of the array and return a reference to the item
	 */
	ValueType& PushBack()
	{
		this->ReserveInternal(this->count + 1);
		ValueType* value = new (this->data + this->count) ValueType;
		++(this->count);
		return *value;
	}


	/**
	 * Add an item to the back of the array
	 */
	void PushBack(const ValueType& value)
	{
		this->ReserveInternal(this->count + 1);
		this->data[this->count] = value;
		++(this->count);
	}

	/**
	 * Insert the specified items to the back of the array
	 */
	void InsertBack(const ValueType* items, SizeType count)
	{
		this->ReserveInternal(this->count + count);

		for (SizeType i = 0; i < count; ++i)
		{
			this->data[this->count] = items[i];
			++(this->count);
		}
	}

	/**
	 * Insert the specified items in the specified position in the array
	 */
	void Insert(SizeType index, const ValueType* items, SizeType itemCount)
	{
		if (index <= count) // Index is valid
		{
			const std::size_t vts = sizeof(ValueType);
			SizeType required = count + itemCount;
			SizeType itemsAfterInsert = itemCount - index;

			if (required > this->allocated) // Requires reallocation
			{
				SizeType newAllocated = this->allocated > 1 ? this->allocated * 2 : 4;

				if (required > newAllocated)
					newAllocated = required;

				ValueType* newData = static_cast<ValueType*>(operator new[](newAllocated * vts));

				// Copy data before inserted items
				std::memcpy(newData, this->data, index * vts);

				// Copy inserted items
				for (unsigned int i = 0; i < itemCount; ++i)
					newData[index + i] = items[i];

				// Copy data after inserted items
				std::memcpy(newData + index + itemCount, this->data + index, itemsAfterInsert * vts);
			}
			else // Can use existing allocated memory
			{
				// Move existing items
				if (itemsAfterInsert > 0)
				{
					ValueType* dst = this->data + index + itemCount;
					ValueType* src = this->data + index;
					std::memmove(dst, src, itemsAfterInsert * vts);
				}

				// Copy inserted items
				for (SizeType i = 0; i < itemCount; ++i)
				{
					this->data[index + i] = items[i];
					++count;
				}
			}
		}
	}

	/**
	 * Remove the last item in the array
	 */
	void PopBack()
	{
		--(this->count);

		this->data[this->count].~ValueType();
	}

	/**
	 * Remove an item from the specified position in the array
	 */
	void Remove(SizeType index)
	{
		this->Remove(index, 1);
	}

	/**
	 * Remove items from the specified position in the array
	 */
	void Remove(SizeType index, SizeType removeCount)
	{
		if (index <= count) // Index is valid
		{
			// Run desctructors
			for (unsigned int i = 0; i < removeCount; ++i)
			{
				this->data[index + i].~ValueType();
				--count;
			}

			SizeType itemsAfterRemove = count - index;

			// Move existing items
			if (itemsAfterRemove > 0)
			{
				ValueType* dst = this->data + index;
				ValueType* src = this->data + index + removeCount;
				std::memmove(dst, src, itemsAfterRemove * sizeof(ValueType));
			}
		}
	}

	/**
	 * Remove all items from the array
	 */
	void Clear()
	{
		for (SizeType i = 0; i < this->count; ++i)
			this->data[i].~ValueType();

		this->count = 0;
	}

	/**
	 * Remove all items from the array and release any allocated memory
	 */
	void ClearAndRelease()
	{
		this->Clear();

		delete[] data;
		data = nullptr;
		allocated = 0;
	}
};
