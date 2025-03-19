#include "Rendering/RenderTypes.hpp"

namespace kokko
{

BufferStorageFlags BufferStorageFlags::None = BufferStorageFlags{};
BufferStorageFlags BufferStorageFlags::Dynamic = BufferStorageFlags{ true, false, false, false, false };

} // namespace kokko
