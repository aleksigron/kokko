#include "Rendering/RenderDevice.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/RenderDeviceDebugScope.hpp"

#ifdef KOKKO_USE_METAL
#include "Rendering/Metal/RenderDeviceMetal.hpp"
#endif

#ifdef KOKKO_USE_OPENGL
#include "Rendering/OpenGL/RenderDeviceOpenGL.hpp"
#endif

#ifdef KOKKO_USE_METAL
RenderDevice* RenderDevice::Create(Allocator* allocator)
{
    return allocator->MakeNew<kokko::RenderDeviceMetal>();
}
#endif

#ifdef KOKKO_USE_OPENGL
RenderDevice* RenderDevice::Create(Allocator* allocator)
{
    return allocator->MakeNew<RenderDeviceOpenGL>();
}
#endif

kokko::RenderDeviceDebugScope RenderDevice::CreateDebugScope(uint32_t id, kokko::ConstStringView message)
{
    return kokko::RenderDeviceDebugScope(this, id, message);
}
