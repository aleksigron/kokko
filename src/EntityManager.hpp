#pragma once

#include "Entity.hpp"
#include "Array.hpp"
#include "Queue.hpp"

class EntityManager
{
private:
	static const unsigned int MinimumFreeIndices = 1024;

	Array<unsigned char> generation;
	Queue<unsigned> freeIndices;

public:
	Entity Create()
	{
		unsigned idx;
		if (freeIndices.GetCount() > MinimumFreeIndices)
		{
			idx = freeIndices.Peek();
			freeIndices.Pop();
		}
		else
		{
			generation.PushBack(0);
			idx = generation.GetCount() - 1;
		}
		return Entity::Make(idx, generation[idx]);
	}

	bool IsAlive(Entity e) const { return generation[e.Index()] == e.Generation(); }

	void Destroy(Entity e)
	{
		const unsigned idx = e.Index();
		++generation[idx];
		freeIndices.Push(idx);
	}
};
