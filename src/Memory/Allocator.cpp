#include "Allocator.hpp"

#include "Memory/DefaultAllocator.hpp"

Allocator* Allocator::GetDefault()
{
    static DefaultAllocator allocator;
    return &allocator;
}
