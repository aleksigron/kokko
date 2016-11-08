#pragma once

#include <cstring>
#include <utility>

template <typename ValueType, typename IndexType = unsigned int>
class IndexedContainer
{
private:
	IndexType* indexList;
	ValueType* objects;

	IndexType objectCount;
	IndexType allocatedCount;
	IndexType freeListFirst;
	IndexType initialAllocation;

	void Reallocate()
	{
		unsigned int newAllocatedCount;

		if (allocatedCount == 0)
			newAllocatedCount = initialAllocation;
		else
			newAllocatedCount = allocatedCount * 2 + 1;

		unsigned int* newIndexList = new unsigned int[newAllocatedCount + 1];
		ValueType* newObjects = new ValueType[newAllocatedCount];

		// We have old data
		if (objectCount > 0)
		{
			// Copy old data to new buffers
			std::memcpy(newIndexList, this->indexList, allocatedCount);
			std::memcpy(newObjects, this->objects, objectCount);

			// Delete old buffers
			delete[] this->indexList;
			delete[] this->objects;
		}

		this->indexList = newIndexList;
		this->objects = newObjects;

		allocatedCount = newAllocatedCount;
	}

public:
	IndexedContainer() :
		indexList(nullptr),
		objects(nullptr),
		objectCount(0),
		allocatedCount(0),
		freeListFirst(0),
		initialAllocation(63)
	{
	}

	~IndexedContainer()
	{
		delete[] indexList;
		delete[] objects;
	}

	void SetInitialAllocation(IndexType size)
	{
		initialAllocation = size;
	}

	IndexType Add()
	{
		IndexType id;

		if (freeListFirst == 0)
		{
			if (objectCount == allocatedCount)
			{
				this->Reallocate();
			}

			// If there are no freelist entries, first <objectCount> indices must be in use
			id = objectCount + 1;
			indexList[id] = objectCount;

		}
		else // There are freelist entries
		{
			id = freeListFirst;
			indexList[id] = objectCount;

			freeListFirst = indexList[freeListFirst];
		}

		++objectCount;

		return id;
	}

	IndexType Add(ValueType** objectPointerOut)
	{
		IndexType id = this->Add();

		*objectPointerOut = objects + objectCount - 1;

		return id;
	}

	void Remove(IndexType id)
	{
		// Put last object in removed objects place
		if (indexList[id] < objectCount - 1)
		{
			objects[indexList[id]] = std::move(objects[objectCount - 1]);
		}

		indexList[id] = freeListFirst;
		freeListFirst = id;
		
		--objectCount;
	}

	ValueType& Get(IndexType id) { return objects[indexList[id]]; }
	const ValueType& Get(IndexType id) const { return objects[indexList[id]]; }

	ValueType* Begin() { return objects; }
	const ValueType* Begin() const { return objects; }

	ValueType* End() { return objects + objectCount; }
	const ValueType* End() const { return objects + objectCount; }
};
