#pragma once

#include <new>
#include <cstring>
#include <utility>

template <typename ValueType, typename IndexType = unsigned int>
class IndexedContainer
{
private:
	void* buffer;
	IndexType* indexList;
	ValueType* objects;

	IndexType count;
	IndexType allocated;
	IndexType freeListFirst;

	void Reallocate(IndexType required)
	{
		if (required <= allocated)
			return;

		unsigned int newAllocation = allocated == 0 ? 8 : allocated * 2;

		if (newAllocation > required)
			required = newAllocation;

		unsigned int its = sizeof(IndexType);
		unsigned int vts = sizeof(ValueType);

		void* newBuffer = operator new[](its + required * (its + vts));
		IndexType* newIndexList = static_cast<IndexType*>(newBuffer);
		ValueType* newObjects = reinterpret_cast<ValueType*>(newIndexList + required + 1);

		// We have old data
		if (this->buffer != nullptr)
		{
			// Copy old data
			unsigned int copy = its * (allocated + 1) + vts * count;
			std::memcpy(newBuffer, this->buffer, copy);

			// Delete old buffers
			operator delete[](this->buffer);
		}

		this->buffer = newBuffer;
		this->indexList = newIndexList;
		this->objects = newObjects;

		allocated = required;
	}

public:
	IndexedContainer() :
		buffer(nullptr),
		indexList(nullptr),
		objects(nullptr),
		count(0),
		allocated(0),
		freeListFirst(0)
	{
	}

	~IndexedContainer()
	{
		operator delete[](buffer);
	}

	IndexType Add()
	{
		IndexType id;

		if (freeListFirst == 0)
		{
			if (count == allocated)
				this->Reallocate(count + 1);

			// If there are no freelist entries, first <objectCount> indices must be in use
			id = count + 1;
			indexList[id] = count;

		}
		else // There are freelist entries
		{
			id = freeListFirst;
			freeListFirst = indexList[freeListFirst];

			indexList[id] = count;
		}

		// Initialize in place
		new (objects + count) ValueType;

		++count;

		return id;
	}

	void Remove(IndexType id)
	{
		// Put last object in removed objects place
		if (indexList[id] < count - 1)
		{
			objects[indexList[id]] = std::move(objects[count - 1]);
		}

		indexList[id] = freeListFirst;
		freeListFirst = id;
		
		--count;
	}

	ValueType& Get(IndexType id) { return objects[indexList[id]]; }
	const ValueType& Get(IndexType id) const { return objects[indexList[id]]; }

	ValueType* Begin() { return objects; }
	const ValueType* Begin() const { return objects; }

	ValueType* End() { return objects + count; }
	const ValueType* End() const { return objects + count; }
};
