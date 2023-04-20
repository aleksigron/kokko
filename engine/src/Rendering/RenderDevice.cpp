#include "Rendering/RenderDevice.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/RenderDeviceDebugScope.hpp"

#ifdef KOKKO_USE_METAL
#include "Rendering/Metal/RenderDeviceMetal.hpp"
#endif

#ifdef KOKKO_USE_OPENGL
#include "Rendering/RenderDeviceOpenGL.hpp"
#endif

namespace kokko
{
namespace render
{

#ifdef KOKKO_USE_METAL
kokko::render::Device* Device::Create(Allocator* allocator)
{
    return allocator->MakeNew<kokko::RenderDeviceMetal>();
}
#endif

#ifdef KOKKO_USE_OPENGL
kokko::render::Device* Device::Create(Allocator* allocator)
{
    return allocator->MakeNew<DeviceOpenGL>();
}
#endif

DeviceDebugScope Device::CreateDebugScope(uint32_t id, kokko::ConstStringView message)
{
    return DeviceDebugScope(this, id, message);
}

} // namespace render
} // namespace kokko
