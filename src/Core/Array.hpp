#pragma once

#include <cstring>
#include <cstddef>
#include <new>

#include "Core/Core.hpp"
#include "Core/ArrayView.hpp"

#include "Memory/Allocator.hpp"

template <typename ValueType>
class Array
{
private:
	Allocator* allocator;
	ValueType* data;
	size_t count;
	size_t allocated;

public:
	Array(Allocator* allocator) :
		allocator(allocator),
		data(nullptr),
		count(0),
		allocated(0)
	{
	}

	~Array()
	{
		for (size_t i = 0; i < count; ++i)
			data[i].~ValueType();

		allocator->Deallocate(this->data);
	}

	size_t GetCount() const { return this->count; }

	ValueType* GetData() { return this->data; }
	const ValueType* GetData() const { return this->data; }

	ValueType& GetFront() { return this->data[0]; }
	const ValueType& GetFront() const { return this->data[0]; }

	ValueType& GetBack() { return this->data[this->count - 1]; }
	const ValueType& GetBack() const { return this->data[this->count - 1]; }

	ValueType& At(size_t index) { return this->data[index]; }
	const ValueType& At(size_t index) const { return this->data[index]; }

	ValueType& operator[](size_t index) { return this->data[index]; }
	const ValueType& operator[](size_t index) const { return this->data[index]; }

	ArrayView<ValueType> GetView() { return ArrayView(data, count); }
	ArrayView<const ValueType> GetView() const { return ArrayView(data, count); }

	ArrayView<ValueType> GetSubView(size_t start, size_t end)
	{
		return GetView().GetSubView(start, end);
	}

	ArrayView<const ValueType> GetSubView(size_t start, size_t end) const
	{
		return GetView().GetSubView(start, end);
	}

	/**
	 * Make sure there's at least the specified amount of space in the array
	 */
	void Reserve(size_t required)
	{
		if (required > allocated)
		{
			size_t newAllocated = allocated > 1 ? allocated * 2 : 4;

			if (required > newAllocated)
				newAllocated = required;

			std::size_t newSize = newAllocated * sizeof(ValueType);
			ValueType* newData = static_cast<ValueType*>(allocator->Allocate(newSize, KOKKO_FUNC_SIG));

			if (data != nullptr)
			{
				if (count > 0) // There is old data
				{
					// Copy old data to new buffer
					std::memcpy(newData, data, count * sizeof(ValueType));
				}

				// Delete old buffer
				allocator->Deallocate(data);
			}

			data = newData;
			allocated = newAllocated;
		}
	}

	/**
	 * Add a new item to the back of the array and return a reference to the item
	 */
	ValueType& PushBack()
	{
		this->Reserve(this->count + 1);
		ValueType* value = new (this->data + this->count) ValueType;
		++(this->count);
		return *value;
	}

	/**
	 * Add an item to the back of the array
	 */
	void PushBack(const ValueType& value)
	{
		this->Reserve(this->count + 1);

		ValueType* ptr = new (this->data + this->count) ValueType;
		*ptr = value;

		++(this->count);
	}

	/**
	 * Insert the specified items to the back of the array
	 */
	void InsertBack(const ValueType* items, size_t count)
	{
		this->Reserve(this->count + count);

		for (size_t i = 0; i < count; ++i)
		{
			this->data[this->count] = items[i];
			++(this->count);
		}
	}

	/**
	 * Insert an item in the specified position in the array
	 */
	void Insert(size_t index, const ValueType& item)
	{
		this->Insert(index, &item, 1);
	}

	/**
	 * Insert items in the specified position in the array
	 */
	void Insert(size_t index, const ValueType* items, size_t itemCount)
	{
		if (index <= count) // Index is valid
		{
			const std::size_t vts = sizeof(ValueType);
			size_t required = count + itemCount;
			size_t itemsAfter = count - index;

			if (required > this->allocated) // Requires reallocation
			{
				size_t newAllocated = this->allocated > 1 ? this->allocated * 2 : 4;

				if (required > newAllocated)
					newAllocated = required;

				std::size_t newSize = newAllocated * vts;
				ValueType* newData = static_cast<ValueType*>(allocator->Allocate(newSize, KOKKO_FUNC_SIG));

				// We have old data
				if (data != nullptr)
				{
					// Copy data before inserted items
					std::memcpy(newData, data, index * vts);

					// Copy data after inserted items
					if (itemsAfter > 0)
						std::memcpy(newData + index + itemCount, data + index, itemsAfter * vts);

					allocator->Deallocate(data);
				}

				// Copy inserted items
				for (unsigned int i = 0; i < itemCount; ++i)
					newData[index + i] = items[i];

				data = newData;
				allocated = newAllocated;
			}
			else // Can use existing allocated memory
			{
				// Move existing items
				if (itemsAfter > 0)
				{
					ValueType* dst = this->data + index + itemCount;
					ValueType* src = this->data + index;
					std::memmove(dst, src, itemsAfter * vts);
				}

				// Copy inserted items
				for (size_t i = 0; i < itemCount; ++i)
					this->data[index + i] = items[i];
			}

			count += itemCount;
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
	void Remove(size_t index)
	{
		this->Remove(index, 1);
	}

	/**
	 * Remove items from the specified position in the array
	 */
	void Remove(size_t index, size_t removeCount)
	{
		if (index <= count) // Index is valid
		{
			// Run desctructors
			for (unsigned int i = 0; i < removeCount; ++i)
			{
				this->data[index + i].~ValueType();
				--count;
			}

			size_t itemsAfterRemove = count - index;

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
	 * Resize the array to have a specific size
	 */
	void Resize(size_t size)
	{
		if (size > count)
		{
			this->Reserve(size);

			ValueType* itr = data + count;
			ValueType* end = data + size;
			for (; itr != end; ++itr)
				new (itr) ValueType;

			count = size;
		}
		else if (size < count)
		{
			this->Remove(size, count - size);
		}
	}

	/**
	 * Remove all items from the array
	 */
	void Clear()
	{
		for (size_t i = 0; i < this->count; ++i)
			this->data[i].~ValueType();

		this->count = 0;
	}

	/**
	 * Remove all items from the array and release any allocated memory
	 */
	void ClearAndRelease()
	{
		this->Clear();

		allocator->Deallocate(data);
		data = nullptr;
		allocated = 0;
	}

	class Iterator
	{
	private:
		ValueType* iterator;

		explicit Iterator(ValueType* itr) : iterator(itr)
		{
		}

		friend class Array;

	public:
		Iterator& operator++()
		{
			iterator++;
			return *this;
		}

		Iterator operator++(int)
		{
			Iterator temp(*this);
			++(*this);
			return temp;
		}

		ValueType& operator*() { return *iterator; }
		const ValueType& operator*() const { return *iterator; }
		ValueType& operator->() { return operator*(); }
		const ValueType& operator->() const { return operator*(); }

		bool operator!=(const Iterator& other) { return iterator != other.iterator; }
	};

	Iterator begin() { return Iterator(data); }
	Iterator end() { return Iterator(data + count); }
};
