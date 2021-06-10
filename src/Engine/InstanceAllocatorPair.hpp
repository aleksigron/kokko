#pragma once

#include "Memory/Allocator.hpp"
#include "Memory/AllocatorManager.hpp"

class Allocator;
class AllocatorManager;

template <typename Type>
struct InstanceAllocatorPair
{
	InstanceAllocatorPair() : instance(nullptr), allocator(nullptr)
	{
	}

	void CreateScope(AllocatorManager* manager, const char* name, Allocator* alloc)
	{
		allocator = manager->CreateAllocatorScope(name, alloc);
	}

	template <typename... Args>
	void New(Args... args)
	{
		instance = allocator->MakeNew<Type>(args...);
	}

	void Delete()
	{
		allocator->MakeDelete(instance);
		instance = nullptr;
	}

	Type* instance;
	Allocator* allocator;
};
