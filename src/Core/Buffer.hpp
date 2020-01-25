#pragma once

#include "Core/BufferRef.hpp"
#include "Memory/Allocator.hpp"

template <typename T>
class Buffer
{
public:
	using SizeType = unsigned long;

private:
	Allocator* allocator;
	T* data;
	SizeType count;
	
public:
	Buffer(Allocator* allocator) :
		allocator(allocator),
		data(nullptr),
		count(0)
	{
	}

	Buffer(const Buffer& other) :
		allocator(nullptr),
		data(nullptr),
		count(0)
	{
		this->operator=(other);
	}

	Buffer(Buffer&& other) :
		allocator(other.allocator),
		data(other.data),
		count(other.count)
	{
		other.data = nullptr;
		other.count = 0;
	}

	~Buffer()
	{
		allocator->Deallocate(data);
	}

	Buffer& operator=(const Buffer& other)
	{
		if (this != &other)
		{
			allocator = other.allocator;

			if (other.data != nullptr && other.count > 0)
			{
				this->Allocate(other.count);

				for (SizeType i = 0; i < count; ++i)
					data[i] = other.data[i];
			}
			else
			{
				data = nullptr;
				count = 0;
			}
		}

		return *this;
	}

	Buffer& operator=(Buffer&& other)
	{
		allocator->Deallocate(data);

		data = other.data;
		count = other.count;

		other.data = nullptr;
		other.count = 0;

		return *this;
	}
	
	void Allocate(SizeType required)
	{
		allocator->Deallocate(data);

		count = required;

		if (required > 0)
		{
			SizeType newSize = required * sizeof(T);
			data = static_cast<T*>(allocator->Allocate(newSize));
		}
		else
			data = nullptr;
	}
	
	void Deallocate()
	{
		allocator->Deallocate(data);
		
		data = nullptr;
		count = 0;
	}

	bool IsValid() { return this->data != nullptr; }

	SizeType Count() const { return count; }

	T* Data() { return data; }
	const T* Data() const { return data; }
	
	T& operator[](SizeType index) { return data[index]; }
	const T& operator[](SizeType index) const { return data[index]; }

	BufferRef<T> GetRef() { return BufferRef<T>(data, count); }
	BufferRef<const T> GetRef() const { return BufferRef<const T>(data, count); }
};
