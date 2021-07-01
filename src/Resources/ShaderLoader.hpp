#pragma once

#include <cstdint>

#include "Core/ArrayView.hpp"

class Allocator;
class RenderDevice;
struct ShaderData;
struct StringRef;

namespace ShaderLoader
{
	bool LoadFromConfiguration(
		ShaderData& shaderOut,
		ArrayView<char> configuration,
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
