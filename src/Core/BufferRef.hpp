#pragma once

template <typename T>
struct BufferRef
{
	using SizeType = unsigned long;

	T* data;
	SizeType count;

	BufferRef() : data(nullptr), count(0)
	{
	}

	BufferRef(T* data, SizeType count) : data(data), count(count)
	{
	}

	bool IsValid()
	{
		return this->data != nullptr;
	}

	T& operator[](SizeType index)
	{
		return this->data[index];
	}

	const T& operator[](SizeType index) const
	{
		return this->data[index];
	}
};
