#include "Rendering/RenderTypes.hpp"

BufferStorageFlags BufferStorageFlags::None = BufferStorageFlags{};
BufferStorageFlags BufferStorageFlags::Dynamic = BufferStorageFlags{ true, false, false, false, false };
