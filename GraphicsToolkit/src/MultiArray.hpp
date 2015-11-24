#pragma once

#include <cstddef>

#include <new>

template <typename T, unsigned int BlockSize = 64>
class MultiArray
{
private:
	using Byte = unsigned char;

	size_t itemCount;

	Byte** arrays;
	size_t arrayCount;
	size_t allocatedArrayCount;

	T* AllocateForPushBack()
	{
		const size_t arrayIndex = itemCount / BlockSize;
		const size_t itemIndex = itemCount % BlockSize;

		// All allocated arrays are full
		if (itemCount == arrayCount * BlockSize)
		{
			// The array list is full
			if (arrayCount == allocatedArrayCount)
			{
				size_t newArrayCount = allocatedArrayCount > 0 ? allocatedArrayCount * 2 : 4;
				Byte** oldArrays = arrays;

				arrays = new Byte*[newArrayCount];

				for (size_t i = 0; i < arrayCount; ++i)
					arrays[i] = oldArrays[i];

				delete[] oldArrays;

				allocatedArrayCount = newArrayCount;
			}

			arrays[arrayIndex] = new Byte[sizeof(T) * BlockSize];
			++arrayCount;
		}

		++itemCount;
		return reinterpret_cast<T**>(arrays)[arrayIndex] + itemIndex;
	}

public:
	MultiArray() :
	itemCount(0),
	arrays(nullptr),
	arrayCount(0),
	allocatedArrayCount(0)
	{
	}

	~MultiArray()
	{
		if (arrays != nullptr)
		{
			for (size_t i = 0; i < arrayCount; ++i)
			{
				T* arr = reinterpret_cast<T**>(arrays)[i];
				size_t c = (i + 1 == arrayCount) ? itemCount % BlockSize : BlockSize;

				// Run destructors
				for (size_t j = 0; j < c; ++j)
					arr[j].~T();

				delete[] arr;
			}

			delete[] arrays;
		}
	}

	inline size_t GetCount() const
	{
		return this->itemCount;
	}

	inline T& At(size_t index)
	{
		return reinterpret_cast<T**>(arrays)[index / BlockSize][index % BlockSize];
	}

	inline const T& At(size_t index) const
	{
		return reinterpret_cast<T**>(arrays)[index / BlockSize][index % BlockSize];
	}

	inline T& operator[](size_t index)
	{
		return reinterpret_cast<T**>(arrays)[index / BlockSize][index % BlockSize];
	}

	inline const T& operator[](size_t index) const
	{
		return reinterpret_cast<T**>(arrays)[index / BlockSize][index % BlockSize];
	}

	void Clear()
	{
		if (arrays != nullptr)
		{
			for (size_t i = 0; i < arrayCount; ++i)
			{
				T* arr = reinterpret_cast<T**>(arrays)[i];
				size_t c = (i + 1 == arrayCount) ? itemCount % BlockSize : BlockSize;

				// Run destructors
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
