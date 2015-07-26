#pragma once

#include <new>

#include <cstdint>
#include <cstddef>

template <typename ValueType, typename IdType, size_t InitialAllocation>
class Collection
{
private:
	uint32_t nextInnerId = 0;
	uint32_t freeList = UINT32_MAX;

	ValueType* objects = nullptr;
	size_t contiguousFree = 0;
	size_t allocatedCount = 0;

public:
	Collection()
	{
		objects = new ValueType[InitialAllocation];
		allocatedCount = InitialAllocation;
	}

	~Collection()
	{
		delete[] objects;
	}

	bool Has(IdType id)
	{
		return objects[id.index].id.innerId == id.innerId;
	}

	ValueType& Get(IdType id)
	{
		return objects[id.index];
	}

	IdType Add()
	{
		IdType id;
		id.innerId = nextInnerId++;

		if (freeList == UINT32_MAX)
		{
			// TODO: Reallocate and copy if contiguousFree == allocatedCount

			// Placement new the RenderObject into the allocated memory
			ValueType* o = new (objects + contiguousFree) ValueType();

			id.index = static_cast<uint32_t>(contiguousFree);
			o->id = id;

			++contiguousFree;
		}
		else
		{
			id.index = freeList;

			// Get pointer to objects[freeList], cast pointer to char*, add sizeof(RenderObjectId) to pointer value, cast pointer to uint32_t*
			uint32_t* next = (uint32_t*)((char*)(&objects[freeList]) + sizeof(IdType));
			freeList = *next;
		}
		
		return id;
	}

	void Remove(IdType id)
	{
		ValueType& o = this->Get(id);
		o.id.innerId = UINT32_MAX;

		// Get pointer to o, cast pointer to char*, add sizeof(RenderObjectId) to pointer value, cast pointer to uint32_t*, dereference pointer
		uint32_t* next = (uint32_t*)((char*)(&o) + sizeof(IdType));
		*next = freeList;

		freeList = id.index;
	}
};