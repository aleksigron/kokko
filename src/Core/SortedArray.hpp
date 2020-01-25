#pragma once

#include "Core/Array.hpp"

template <typename ValueType>
class SortedArray
{
private:
	Array<ValueType> a;

public:
	using SizeType = typename Array<ValueType>::SizeType;

	SortedArray(Allocator* allocator) :
		a(allocator)
	{
	}

	SizeType GetCount() const { return a.GetCount(); }

	ValueType* GetData() { return a.GetData(); }
	const ValueType* GetData() const { return a.GetData(); }

	ValueType& operator[](SizeType index) { return a[index]; }
	const ValueType& operator[](SizeType index) const { return a[index]; }

	void Insert(const ValueType* items, SizeType count)
	{
		for (const ValueType* end = items + count; items != end; ++items)
			this->Insert(*items);
	}

	void Insert(const ValueType& val)
	{
		SizeType count = a.GetCount();
		SizeType insertIndex = 0;

		if (count > 0)
		{
			const ValueType* data = a.GetData();
			SizeType l = 0;
			SizeType r = count - 1;

			while (l != r)
			{
				SizeType m = (l + r + 1) / 2;

				if (data[m] > val)
					r = m - 1;
				else
					l = m;
			}

			insertIndex = l;

			if (data[l] < val)
				++insertIndex;
		}

		a.Insert(insertIndex, val);
	}

	void InsertUnique(const ValueType* items, SizeType count)
	{
		for (const ValueType* end = items + count; items != end; ++items)
			this->InsertUnique(*items);
	}

	void InsertUnique(const ValueType& val)
	{
		SizeType count = a.GetCount();
		SizeType insertIndex = 0;

		if (count > 0)
		{
			const ValueType* data = a.GetData();
			SizeType l = 0;
			SizeType r = count - 1;

			while (l != r)
			{
				SizeType m = (l + r + 1) / 2;

				if (data[m] > val)
					r = m - 1;
				else
					l = m;
			}

			insertIndex = l;

			if (data[l] < val)
				a.Insert(l + 1, val);
			else if (data[l] == val == false)
				a.Insert(l, val);
		}
		else
			a.PushBack(val);
	}

	void Clear()
	{
		a.Clear();
	}
};
