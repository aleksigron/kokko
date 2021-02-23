#pragma once

#include "Core/Array.hpp"

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
private:
	static constexpr size_t KernelSize = 64;
	static constexpr unsigned int NoiseTextureSize = 4;
	static constexpr unsigned int UniformBlockBinding = 0;

	struct PassInfo
	{
		unsigned int framebufferId;
		unsigned int textureId;
	};

	struct UniformBlock
	{
		UniformBlockArray<Vec3f, KernelSize> kernel;
		alignas(16) Mat4x4f projection;
		alignas(8) Vec2f halfNearPlane;
		alignas(8) Vec2f noiseScale;
		alignas(4) float sampleRadius;
	};

	Allocator* allocator;
	RenderDevice* renderDevice;
	ShaderManager* shaderManager;

	Array<Vec3f> kernel;

	Vec2i framebufferSize;

	ShaderId occlusionShaderId;
	ShaderId blurShaderId;

	PassInfo occlusionPass;
	PassInfo blurPass;
	unsigned int uniformBufferId;
	unsigned int noiseTextureId;

	void CreatePassResources(PassInfo& passInfoOut);
	void DestroyPassResources(PassInfo& passInfoInOut);

public:
	ScreenSpaceAmbientOcclusion(Allocator* allocator, RenderDevice* renderDevice, ShaderManager* shaderManager);
	~ScreenSpaceAmbientOcclusion();

	void Initialize(Vec2i framebufferResolution);
	void Deinitialize();

	unsigned int GetOcclusionFramebufferId() const { return occlusionPass.framebufferId; }
	unsigned int GetOcclusionTextureId() const { return occlusionPass.textureId; }

	unsigned int GetBlurFramebufferId() const { return blurPass.framebufferId; }
	unsigned int GetBlurTextureId() const { return blurPass.textureId; }

	unsigned int GetNoiseTextureId() const { return noiseTextureId; }
	unsigned int GetUniformBufferBindingPoint() const { return UniformBlockBinding; }

	void UpdateUniformBuffer(const ProjectionParameters& projection) const;
	unsigned int GetUniformBufferId() const { return uniformBufferId; }

	ShaderId GetOcclusionShaderId() const { return occlusionShaderId; }
	ShaderId GetBlurShaderId() const { return blurShaderId; }
};
