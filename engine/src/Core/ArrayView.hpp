#pragma once

#include <cstddef>

namespace kokko
{

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

	template <size_t Count>
	ArrayView(ValueType(&data)[Count]) : data(data), count(Count)
	{
	}

	ValueType& operator[](size_t index) { return data[index]; }
	const ValueType& operator[](size_t index) const { return data[index]; }

	size_t GetCount() const { return count; }

	ValueType* GetData() { return data; }
	const ValueType* GetData() const { return data; }

	ArrayView<ValueType> GetSubView(size_t start, size_t end) const
	{
		return ArrayView(&data[start], end - start);
	}

	ArrayView<ValueType>& TrimBeginning(size_t amount)
	{
		data += amount;
		count -= amount;
		return *this;
	}

	ArrayView<ValueType>& TrimEnd(size_t amount)
	{
		count -= amount;
		return *this;
	}

	ValueType* Find(const ValueType& value) const
	{
		ValueType* itr = begin();
		ValueType* end_ = end();
		for (; itr != end_; ++itr)
			if (*itr == value)
				return itr;
	}

	template <typename Compare>
	ValueType* FindIf(const Compare& compare)
	{
		ValueType* itr = begin();
		ValueType* end_ = end();
		for (; itr != end_; ++itr)
			if (compare(*itr))
				return itr;

		return nullptr;
	}

	ValueType* begin() { return data; }
	const ValueType* begin() const { return data; }

	ValueType* end() { return &data[count]; }
	const ValueType* end() const { return &data[count]; }

	operator ArrayView<const ValueType>() const
	{
		return ArrayView<const ValueType>(data, count);
	}

private:
	ValueType* data;
	size_t count;
};

} // namespace kokko
