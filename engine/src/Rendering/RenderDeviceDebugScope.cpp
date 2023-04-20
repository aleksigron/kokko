#include "Rendering/RenderDeviceDebugScope.hpp"

#include "Rendering/RenderDevice.hpp"

namespace kokko
{

RenderDeviceDebugScope::RenderDeviceDebugScope(
	RenderDevice* renderDevice, uint32_t id, kokko::ConstStringView message) :
	renderDevice(renderDevice)
{
	renderDevice->BeginDebugScope(id, message);
}

RenderDeviceDebugScope::RenderDeviceDebugScope(RenderDeviceDebugScope&& other) noexcept
{
	renderDevice = other.renderDevice;
	other.renderDevice = nullptr;
}

RenderDeviceDebugScope::~RenderDeviceDebugScope()
{
	if (renderDevice != nullptr)
		renderDevice->EndDebugScope();
}

RenderDeviceDebugScope& RenderDeviceDebugScope::operator=(RenderDeviceDebugScope&& other) noexcept
{
	renderDevice = other.renderDevice;
	other.renderDevice = nullptr;

	return *this;
}

} // namespace kokko
