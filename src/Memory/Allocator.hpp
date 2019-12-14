#pragma once

#include <cstdio>

class Allocator
{
public:
	virtual void* Allocate(std::size_t size) = 0;
	virtual void Deallocate(void* p) = 0;

	template <typename T, typename... Args>
	T* MakeNew(Args... args) { return new (Allocate(sizeof(T))) T(args...); }

	template <typename T>
	void MakeDelete(T* p)
	{
		if (p != nullptr)
		{
			p->~T();
			Deallocate(p);
		}
	}
};
