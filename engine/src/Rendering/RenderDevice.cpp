#include "Rendering/RenderDevice.hpp"

#include "Memory/Allocator.hpp"

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
