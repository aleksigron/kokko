#pragma once

#include <cstddef>

#include <new>

template <typename T, unsigned int BlockSize = 64>
class MultiArray
{
private:
	size_t itemCount;

	T** arrays;
	size_t arrayCount;
	size_t allocArrayCount;

	T* AllocateForPushBack()
	{
		const size_t arrayIndex = itemCount / BlockSize;
		const size_t itemIndex = itemCount % BlockSize;

		// All allocated arrays are full
		if (itemCount == arrayCount * BlockSize)
		{
			// The array list is full
			if (arrayCount == allocArrayCount)
			{
				// New array list size
				size_t newArrayCount = allocArrayCount > 0 ? allocArrayCount * 2 : 4;
				T** oldArrays = arrays;

				// Allocate a new array list
				arrays = static_cast<T**>(operator new(sizeof(T*) * newArrayCount));

				// Copy the old array pointers to new array list
				for (size_t i = 0; i < arrayCount; ++i)
					arrays[i] = oldArrays[i];

				// Deallocate old array list
				operator delete(oldArrays);

				allocArrayCount = newArrayCount;
			}

			// Allocate a new element array
			arrays[arrayIndex] = static_cast<T*>(operator new(sizeof(T) * BlockSize));
			++arrayCount;
		}

		++itemCount;
		return arrays[arrayIndex] + itemIndex;
	}

public:
	MultiArray() :
	itemCount(0),
	arrays(nullptr),
	arrayCount(0),
	allocArrayCount(0)
	{
	}

	~MultiArray()
	{
		// We have allocated some storage
		if (arrays != nullptr)
		{
			// For each used array in the array list
			for (size_t i = 0; i < arrayCount; ++i)
			{
				T* arr = arrays[i];

				// The number of in-use elements in this array
				size_t c = (i + 1 == arrayCount) ? itemCount % BlockSize : BlockSize;

				// Run destructors for each element
				for (size_t j = 0; j < c; ++j)
					arr[j].~T();

				// Deallocate the array
				operator delete(arr);
			}

			// Deallocate the array list
			operator delete(arrays);
		}
	}

	inline size_t GetCount() const
	{
		return this->itemCount;
	}

	inline T& At(size_t index)
	{
		return arrays[index / BlockSize][index % BlockSize];
	}

	inline const T& At(size_t index) const
	{
		return arrays[index / BlockSize][index % BlockSize];
	}

	inline T& operator[](size_t index)
	{
		return arrays[index / BlockSize][index % BlockSize];
	}

	inline const T& operator[](size_t index) const
	{
		return arrays[index / BlockSize][index % BlockSize];
	}

	void Clear()
	{
		// We have allocated some storage
		if (arrays != nullptr)
		{
			// For each used array in the array list
			for (size_t i = 0; i < arrayCount; ++i)
			{
				T* arr = arrays[i];

				// The number of in-use elements in this array
				size_t c = (i + 1 == arrayCount) ? itemCount % BlockSize : BlockSize;

				// Run destructors for each element
				for (size_t j = 0; j < c; ++j)
					arr[j].~T();
			}
		}

		itemCount = 0;
	}

	void PushBack(const T& item)
	{
		T* ptr = this->AllocateForPushBack();
		*ptr = item;
	}

	T& PushBack()
	{
		T* ptr = this->AllocateForPushBack();
		return *(new (ptr) T());
	}

	void PopBack()
	{
		this->At(itemCount--).~T();
	}
};
