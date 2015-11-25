#pragma once

#include <new>

template <typename T, unsigned int BlockSize = 64>
class MultiArray
{
public:
	using SizeType = unsigned long;

private:
	SizeType itemCount;

	T** arrays;
	SizeType arrayCount;
	SizeType allocArrayCount;

	T* AllocateForPushBack()
	{
		const SizeType arrayIndex = itemCount / BlockSize;
		const SizeType itemIndex = itemCount % BlockSize;

		// All allocated arrays are full
		if (itemCount == arrayCount * BlockSize)
		{
			// The array list is full
			if (arrayCount == allocArrayCount)
			{
				// New array list size
				SizeType newArrayCount = allocArrayCount > 0 ? allocArrayCount * 2 : 4;
				T** oldArrays = arrays;

				// Allocate a new array list
				arrays = static_cast<T**>(operator new(sizeof(T*) * newArrayCount));

				// Copy the old array pointers to new array list
				for (SizeType i = 0; i < arrayCount; ++i)
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
			for (SizeType i = 0; i < arrayCount; ++i)
			{
				T* arr = arrays[i];

				// The number of in-use elements in this array
				SizeType c = (i + 1 == arrayCount) ? itemCount % BlockSize : BlockSize;

				// Run destructors for each element
				for (SizeType j = 0; j < c; ++j)
					arr[j].~T();

				// Deallocate the array
				operator delete(arr);
			}

			// Deallocate the array list
			operator delete(arrays);
		}
	}

	inline SizeType GetCount() const
	{
		return this->itemCount;
	}

	inline T& At(SizeType index)
	{
		return arrays[index / BlockSize][index % BlockSize];
	}

	inline const T& At(SizeType index) const
	{
		return arrays[index / BlockSize][index % BlockSize];
	}

	inline T& operator[](SizeType index)
	{
		return arrays[index / BlockSize][index % BlockSize];
	}

	inline const T& operator[](SizeType index) const
	{
		return arrays[index / BlockSize][index % BlockSize];
	}

	void Clear()
	{
		// We have allocated some storage
		if (arrays != nullptr)
		{
			// For each used array in the array list
			for (SizeType i = 0; i < arrayCount; ++i)
			{
				T* arr = arrays[i];

				// The number of in-use elements in this array
				SizeType c = (i + 1 == arrayCount) ? itemCount % BlockSize : BlockSize;

				// Run destructors for each element
				for (SizeType j = 0; j < c; ++j)
					arr[j].~T();
			}
		}

		itemCount = 0;
	}

	// TODO: void Insert(T* array, SizeType count);

	// Insert an object at the end of the container
	void PushBack(const T& item)
	{
		T* ptr = this->AllocateForPushBack();

		// Assign the new item
		*ptr = item;
	}

	// Insert a default-constructed object at the end of the container
	T& PushBack()
	{
		T* ptr = this->AllocateForPushBack();

		// Construct the object in-place
		new (ptr) T();

		return *ptr;
	}

	// Remove the last object in the container
	void PopBack()
	{
		// Run destructor on last object
		this->At(itemCount - 1).~T();

		--itemCount;
	}
};
