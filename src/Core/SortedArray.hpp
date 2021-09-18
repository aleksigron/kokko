#pragma once

#include <cstdint>

#include "Core/Array.hpp"

template <typename ValueType>
class SortedArray
{
private:
	Array<ValueType> a;

	/*
	Tries to find value. If it's not found, returns the index at which the value
	should be inserted. Array must be checked to not be empty before calling.
	 */
	size_t FindInternal(const ValueType& value) const
	{
		size_t index = 0;

		const ValueType* data = a.GetData();
		size_t l = 0;
		size_t r = a.GetCount() - 1;

		while (l != r)
		{
			size_t m = (l + r + 1) / 2;

			if (data[m] > value)
				r = m - 1;
			else
				l = m;
		}

		index = l;

		if (data[index] < value)
			++index;

		return index;
	}

public:
	SortedArray(Allocator* allocator) :
		a(allocator)
	{
	}

	size_t GetCount() const { return a.GetCount(); }

	ValueType* GetData() { return a.GetData(); }
	const ValueType* GetData() const { return a.GetData(); }

	ValueType& operator[](size_t index) { return a[index]; }
	const ValueType& operator[](size_t index) const { return a[index]; }

	bool Contains(const ValueType& value) const
	{
		if (a.GetCount() == 0)
			return false;

		size_t index = FindInternal(value);

		return index < a.GetCount() && a[index] == value;
	}

	intptr_t Find(const ValueType& value) const
	{
		if (a.GetCount() == 0)
			return -1;

		size_t index = FindInternal(value);

		if (a[index] == value)
			return index;
		else
			return -1;
	}

	void Insert(const ValueType* items, size_t count)
	{
		for (const ValueType* end = items + count; items != end; ++items)
			this->Insert(*items);
	}

	void Insert(const ValueType& val)
	{
		size_t insertIndex = 0;

		if (a.GetCount() > 0)
			insertIndex = FindInternal(val);

		a.Insert(insertIndex, val);
	}

	void InsertUnique(const ValueType* items, size_t count)
	{
		for (const ValueType* end = items + count; items != end; ++items)
			this->InsertUnique(*items);
	}

	void InsertUnique(const ValueType& val)
	{
		if (a.GetCount() > 0)
		{
			size_t index = FindInternal(val);

			if (index >= a.GetCount() || !(a[index] == val))
				a.Insert(index, val);
		}
		else
			a.PushBack(val);
	}

	void PopBack()
	{
		a.PopBack();
	}

	void Remove(size_t index)
	{
		a.Remove(index);
	}

	void Clear()
	{
		a.Clear();
	}

	typename Array<ValueType>::Iterator begin() { return a.begin(); }
	typename Array<ValueType>::Iterator end() { return a.end(); }
};
