#pragma once

#include <cstdint>

#include "Core/StringView.hpp"

namespace kokko
{
namespace render
{

class Device;

class DeviceDebugScope
{
public:
	DeviceDebugScope(Device* renderDevice, uint32_t id, kokko::ConstStringView message);
	DeviceDebugScope(const DeviceDebugScope&) = delete;
	DeviceDebugScope(DeviceDebugScope&& other) noexcept;
	~DeviceDebugScope();

	DeviceDebugScope& operator=(const DeviceDebugScope&) = delete;
	DeviceDebugScope& operator=(DeviceDebugScope&& other) noexcept;

private:
	Device* renderDevice;
};

} // namespace render
} // namespace kokko
