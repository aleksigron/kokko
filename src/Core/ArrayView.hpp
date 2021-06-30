#pragma once

#include <cstdint>

template <typename ValueType>
class ArrayView
{
public:
	ArrayView() : data(nullptr), count(0)
	{
	}

	ArrayView(ValueType* data, size_t count) : data(data), count(count)
	{
	}

	ValueType& operator[](size_t index) { return data[index]; }
	const ValueType& operator[](size_t index) const { return data[index]; }

	size_t GetCount() const { return count; }

	ValueType* GetData() { return data; }
	const ValueType* GetData() const { return data; }

	ArrayView<ValueType> GetSubView(size_t start, size_t end)
	{
		return ArrayView(&data[start], end - start);
	}

	ArrayView<const ValueType> GetSubView(size_t start, size_t end) const
	{
		return ArrayView(&data[start], end - start);
	}

	ValueType* begin() { return data; }
	const ValueType* begin() const { return data; }

	ValueType* end() { return &data[count]; }
	const ValueType* end() const { return &data[count]; }

private:
	ValueType* data;
	size_t count;
};
