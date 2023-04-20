#include "Rendering/RenderDeviceDebugScope.hpp"

#include "Rendering/RenderDevice.hpp"

namespace kokko
{
namespace render
{

DeviceDebugScope::DeviceDebugScope(
	Device* renderDevice, uint32_t id, kokko::ConstStringView message) :
	renderDevice(renderDevice)
{
	renderDevice->BeginDebugScope(id, message);
}

DeviceDebugScope::DeviceDebugScope(DeviceDebugScope&& other) noexcept
{
	renderDevice = other.renderDevice;
	other.renderDevice = nullptr;
}

DeviceDebugScope::~DeviceDebugScope()
{
	if (renderDevice != nullptr)
		renderDevice->EndDebugScope();
}

DeviceDebugScope& DeviceDebugScope::operator=(DeviceDebugScope&& other) noexcept
{
	renderDevice = other.renderDevice;
	other.renderDevice = nullptr;

	return *this;
}

} // namespace render
} // namespace kokko
