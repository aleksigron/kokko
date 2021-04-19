#pragma once

#include "Core/Array.hpp"

template <typename ValueType>
class SortedArray
{

public:
	using SizeType = typename Array<ValueType>::SizeType;

private:
	Array<ValueType> a;

	/*
	Tries to find value. If it's not found, returns the index at which the value
	should be inserted. Array must be checked to not be empty before calling.
	 */
	SizeType FindInternal(const ValueType& value) const
	{
		SizeType index = 0;

		const ValueType* data = a.GetData();
		SizeType l = 0;
		SizeType r = a.GetCount() - 1;

		while (l != r)
		{
			SizeType m = (l + r + 1) / 2;

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

	SizeType GetCount() const { return a.GetCount(); }

	ValueType* GetData() { return a.GetData(); }
	const ValueType* GetData() const { return a.GetData(); }

	ValueType& operator[](SizeType index) { return a[index]; }
	const ValueType& operator[](SizeType index) const { return a[index]; }

	bool Contains(const ValueType& value) const
	{
		if (a.GetCount() == 0)
			return false;

		SizeType index = FindInternal(value);

		return a[index] == value;
	}

	void Insert(const ValueType* items, SizeType count)
	{
		for (const ValueType* end = items + count; items != end; ++items)
			this->Insert(*items);
	}

	void Insert(const ValueType& val)
	{
		SizeType insertIndex = 0;

		if (a.GetCount() > 0)
			insertIndex = FindInternal(val);

		a.Insert(insertIndex, val);
	}

	void InsertUnique(const ValueType* items, SizeType count)
	{
		for (const ValueType* end = items + count; items != end; ++items)
			this->InsertUnique(*items);
	}

	void InsertUnique(const ValueType& val)
	{
		if (a.GetCount() > 0)
		{
			SizeType index = FindInternal(val);

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

	void Clear()
	{
		a.Clear();
	}
};
