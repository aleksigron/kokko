#pragma once

#include <cstdint>

#include "Core/BufferRef.hpp"

class Allocator;
class RenderDevice;
struct ShaderData;

namespace ShaderLoader
{
	bool LoadFromConfiguration(
		ShaderData& shaderOut,
		BufferRef<char> configuration,
		Allocator* allocator,
		RenderDevice* renderDevice);
}
