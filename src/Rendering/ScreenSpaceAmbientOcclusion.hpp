#pragma once

#include "Core/Array.hpp"
#include "Core/StringRef.hpp"

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Mat4x4.hpp"
#include "Math/Projection.hpp"

#include "Rendering/StaticUniformBuffer.hpp"

#include "Resources/ShaderId.hpp"

class Allocator;
class RenderDevice;
class ShaderManager;
class PostProcessRenderer;

struct RenderTarget;

class ScreenSpaceAmbientOcclusion
{
public:
	struct RenderParams
	{
		unsigned int normalTexture;
		unsigned int depthTexture;
		ProjectionParameters projection;
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
		alignas(16) Vec2f textureScale;
	};

	Allocator* allocator;
	RenderDevice* renderDevice;
	ShaderManager* shaderManager;
	PostProcessRenderer* postProcessRenderer;

	unsigned int resultRenderTargetId;
	unsigned int resultTextureId;

	Array<Vec3f> kernel;

	Vec2i framebufferSize;
	int kernelSize;

	static const size_t PassCount = 2;
	enum { PassIdx_Occlusion = 0, PassIdx_Blur = 1 };

	unsigned int uniformBufferIds[PassCount];
	ShaderId shaderIds[PassCount];

	unsigned int noiseTextureId;

	void CreatePassResources(size_t passCount,
		const StringRef* shaderPaths,
		const size_t* uniformSizes);

	void UpdateUniformBuffers(const ProjectionParameters& projection) const;

public:
	ScreenSpaceAmbientOcclusion(
		Allocator* allocator,
		RenderDevice* renderDevice,
		ShaderManager* shaderManager,
		PostProcessRenderer* postProcessRenderer);

	~ScreenSpaceAmbientOcclusion();

	void Initialize(Vec2i framebufferResolution);
	void Deinitialize();

	void Render(const RenderParams& params);

	unsigned int GetResultTextureId();
	void ReleaseResult();
};
