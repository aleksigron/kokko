#pragma once

#include "Core/Array.hpp"
#include "Core/StringRef.hpp"

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Mat4x4.hpp"

#include "Rendering/StaticUniformBuffer.hpp"

#include "Resources/ShaderId.hpp"

class Allocator;
class RenderDevice;
class ShaderManager;
struct ProjectionParameters;

class ScreenSpaceAmbientOcclusion
{
public:
	struct PassInfo
	{
		unsigned int framebufferId;
		unsigned int textureId;
		unsigned int uniformBufferId;
		ShaderId shaderId;
	};

private:
	static constexpr size_t MaxKernelSize = 64;
	static constexpr unsigned int NoiseTextureSize = 4;
	static constexpr unsigned int UniformBlockBinding = 0;

	struct OcclusionUniformBlock
	{
		UniformBlockArray<Vec3f, MaxKernelSize> kernel;
		alignas(16) Mat4x4f projection;
		alignas(8) Vec2f halfNearPlane;
		alignas(8) Vec2f noiseScale;
		alignas(4) float sampleRadius;
		alignas(4) int kernelSize;
	};

	struct BlurUniformBlock
	{
		alignas(8) Vec2f textureScale;
	};

	Allocator* allocator;
	RenderDevice* renderDevice;
	ShaderManager* shaderManager;

	Array<Vec3f> kernel;

	Vec2i framebufferSize;
	int kernelSize;

	PassInfo occlusionPass;
	PassInfo blurPass;
	unsigned int noiseTextureId;

	void CreatePassResources(StringRef shaderPath, size_t uniformSize, PassInfo& passInfoOut);
	void DestroyPassResources(PassInfo& passInfoInOut);

public:
	ScreenSpaceAmbientOcclusion(Allocator* allocator, RenderDevice* renderDevice, ShaderManager* shaderManager);
	~ScreenSpaceAmbientOcclusion();

	void Initialize(Vec2i framebufferResolution);
	void Deinitialize();

	const PassInfo& GetOcclusionPassInfo() const { return occlusionPass; }
	void UpdateOcclusionUniformBuffer(const ProjectionParameters& projection) const;

	const PassInfo& GetBlurPassInfo() const { return blurPass; }
	void UpdateBlurUniformBuffer() const;

	unsigned int GetNoiseTextureId() const { return noiseTextureId; }
	unsigned int GetUniformBufferBindingPoint() const { return UniformBlockBinding; }
};
