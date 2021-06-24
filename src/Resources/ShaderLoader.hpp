#pragma once

#include <cstdint>

#include "Core/BufferRef.hpp"

class Allocator;
class RenderDevice;
struct ShaderData;
struct StringRef;

namespace ShaderLoader
{
	bool LoadFromConfiguration(
		ShaderData& shaderOut,
		BufferRef<char> configuration,
		Allocator* allocator,
		RenderDevice* renderDevice,
		StringRef debugName);

	bool LoadFromShaderFile(
		ShaderData& shaderOut,
		StringRef shaderPath,
		StringRef shaderContent,
		Allocator* allocator,
		RenderDevice* renderDevice,
		StringRef debugName);
}
