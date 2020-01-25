#pragma once

#include "Core/BufferRef.hpp"

template <typename T>
class Buffer
{
public:
	using SizeType = unsigned long;

private:
	T* data;
	SizeType count;
	
public:
	Buffer(): data(nullptr), count(0)
	{
	}

	Buffer(const Buffer& other): data(nullptr), count(0)
	{
		this->operator=(other);
	}

	Buffer(Buffer&& other): data(other.data), count(other.count)
	{
		other.data = nullptr;
		other.count = 0;
	}

	~Buffer()
	{
		delete[] data;
	}

	Buffer& operator=(const Buffer& other)
	{
		if (this != &other)
		{
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
		delete[] data;

		data = other.data;
		count = other.count;

		other.data = nullptr;
		other.count = 0;

		return *this;
	}
	
	void Allocate(SizeType required)
	{
		delete[] data;

		count = required;

		if (required > 0)
			data = new T[required];
		else
			data = nullptr;
	}
	
	void Deallocate()
	{
		delete[] data;
		
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
