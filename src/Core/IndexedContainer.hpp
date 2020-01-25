#pragma once

#include <cstring>

template <typename ValueType>
class IndexedContainer
{
public:
	using IndexType = unsigned int;

private:
	static_assert(sizeof(ValueType) >= sizeof(IndexType),
		"ValueType must be at least the same size as unsigned int");

	ValueType* data;

	IndexType count;
	IndexType allocated;
	IndexType freeListFirst;

	IndexType* GetFreeListSlot(IndexType index)
	{
		return reinterpret_cast<IndexType*>(data + index);
	}

	void Reallocate(IndexType required)
	{
		if (required > allocated)
		{
			unsigned int newAlloc = allocated == 0 ? 8 : allocated * 2;

			if (required > newAlloc)
				newAlloc = required;

			ValueType* newData = static_cast<ValueType*>(operator new[](newAlloc * sizeof(ValueType)));

			// We have old data
			if (data != nullptr)
			{
				// Copy old data
				std::memcpy(newData, data, sizeof(ValueType) * allocated);

				operator delete[](data);
			}

			data = newData;
			allocated = required;
		}
	}

public:
	IndexedContainer() :
		data(nullptr),
		count(0),
		allocated(0),
		freeListFirst(0)
	{
	}

	~IndexedContainer()
	{
		operator delete[](data);
	}

	IndexType Add()
	{
		IndexType id;

		if (freeListFirst == 0)
		{
			if (count == allocated)
				this->Reallocate(count + 1);

			// If there are no freelist entries, first <count> indices must be in use
			id = count + 1;
		}
		else // There are freelist entries
		{
			// We can use a freelist position again
			id = freeListFirst;

			// Index into the freelist is always one less than the public index
			IndexType* freeListSlot = GetFreeListSlot(freeListFirst - 1);

			// Move freeListFirst index to next link in chain
			// If this was the only item in the freelist, <freeListFirst> will be 0
			freeListFirst = *freeListSlot;
		}

		++count;

		return id;
	}

	void Remove(IndexType id)
	{
		// Index into the freelist is always one less than the public index
		IndexType* freeListSlot = GetFreeListSlot(id - 1);

		// If there currently are no freelist items, <freeListFirst> is 0
		*freeListSlot = freeListFirst;

		// The removed item is now the first freelist item
		freeListFirst = id;
		
		--count;
	}

	ValueType& Get(IndexType id) { return data[id - 1]; }
	const ValueType& Get(IndexType id) const { return data[id - 1]; }
};
