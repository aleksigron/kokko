#pragma once

#include <cstdint>

#include "Core/StringView.hpp"

class RenderDevice;

namespace kokko
{

class RenderDeviceDebugScope
{
public:
	RenderDeviceDebugScope(RenderDevice* renderDevice, uint32_t id, kokko::ConstStringView message);
	RenderDeviceDebugScope(const RenderDeviceDebugScope&) = delete;
	RenderDeviceDebugScope(RenderDeviceDebugScope&& other) noexcept;
	~RenderDeviceDebugScope();

	RenderDeviceDebugScope& operator=(const RenderDeviceDebugScope&) = delete;
	RenderDeviceDebugScope& operator=(RenderDeviceDebugScope&& other) noexcept;

private:
	RenderDevice* renderDevice;
};

}
