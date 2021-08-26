#pragma once

#include <cstdint>
#include <new>

class Allocator
{
public:
	virtual ~Allocator() {}

	virtual void* Allocate(size_t size, const char* debugTag = nullptr) = 0;
	virtual void* AllocateAligned(size_t size, size_t alignment, const char* debugTag = nullptr) = 0;
	virtual void Deallocate(void* ptr) = 0;
	virtual size_t GetAllocatedSize(void* ptr) = 0;

	template <typename T, typename... Args>
	T* MakeNew(Args... args)
	{
		return new (Allocate(sizeof(T), __FUNCSIG__)) T(args...);
	}

	template <typename T>
	void MakeDelete(T* p)
	{
		if (p != nullptr)
		{
			p->~T();
			Deallocate(p);
		}
	}

	static Allocator* GetDefault();
};
