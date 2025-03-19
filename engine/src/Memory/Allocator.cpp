#include "Allocator.hpp"

#include "Memory/DefaultAllocator.hpp"

namespace kokko
{

Allocator* Allocator::GetDefault()
{
    static DefaultAllocator allocator;
    return &allocator;
}

} // namespace kokko
