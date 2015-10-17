#include "StackAllocation.hpp"

#include "StackAllocator.hpp"

StackAllocation::StackAllocation()
{
}

StackAllocation::StackAllocation(StackAllocation&& other)
{
	this->allocator = other.allocator;
	this->data = other.data;
	this->size = other.size;

	other.allocator = nullptr;
	other.data = nullptr;
	other.size = 0;
}

StackAllocation::~StackAllocation()
{
	if (this->allocator != nullptr && this->data != nullptr)
	{
		this->allocator->Deallocate(*this);
	}
}
