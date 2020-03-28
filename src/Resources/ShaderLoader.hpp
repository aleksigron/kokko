#pragma once

#include <cstdint>

#include "Core/BufferRef.hpp"

class Allocator;
class RenderDevice;
struct Shader;

namespace ShaderLoader
{
	bool LoadFromConfiguration(
		Shader& shaderOut,
		BufferRef<char> configuration,
		Allocator* allocator,
		RenderDevice* renderDevice);
}
