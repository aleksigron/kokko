#pragma once

#include "Memory/Allocator.hpp"

namespace kokko
{

template <typename T>
class UniquePtr
{
public:
	UniquePtr() : allocator(nullptr), ptr(nullptr) { }
	UniquePtr(Allocator* allocator, T* ptr) : allocator(allocator), ptr(ptr) { }
	UniquePtr(const UniquePtr&) = delete;
	UniquePtr(UniquePtr&& other)
	{
		ptr = other.ptr;
		allocator = other.allocator;

		other.ptr = nullptr;
		other.allocator = nullptr;
	}

	~UniquePtr()
	{
		if (allocator)
			allocator->MakeDelete(ptr);
	}

	UniquePtr& operator=(const UniquePtr&) = delete;
	UniquePtr& operator=(UniquePtr&& other)
	{
		if (allocator != nullptr)
		{
			allocator->MakeDelete(this->ptr);
		}

		ptr = other.ptr;
		allocator = other.allocator;

		other.ptr = nullptr;
		other.allocator = nullptr;

		return *this;
	}

	T& operator*() { return *ptr; }
	const T& operator*() const { return *ptr; }
	T* operator->() { return ptr; }
	const T* operator->() const { return ptr; }

	bool operator==(nullptr_t) const { return ptr == nullptr; }
	bool operator!=(nullptr_t) const { return ptr != nullptr; }

	T* Get() { return ptr; }
	const T* Get() const { return ptr; }

private:
	Allocator* allocator;
	T* ptr;
};

template <typename T, typename... Args>
inline UniquePtr<T> MakeUnique(Allocator* allocator, Args... constructArgs)
{
	return UniquePtr(allocator, new (allocator->Allocate(sizeof(T), KOKKO_FUNC_SIG)) T(constructArgs...));
}

} // namespace kokko
