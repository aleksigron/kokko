#pragma once

#include "Math/Vec2.hpp"

#include "Resources/ShaderId.hpp"

class Allocator;
class RenderDevice;
class PostProcessRenderer;
class RenderTargetContainer;
class ShaderManager;

class BloomEffect
{
private:
	Allocator* allocator;
	RenderDevice* renderDevice;
	PostProcessRenderer* postProcessRenderer;
	RenderTargetContainer* renderTargetContainer;
	ShaderManager* shaderManager;

	unsigned char* uniformStagingBuffer;

	ShaderId extractShaderId;
	ShaderId downsampleShaderId;
	ShaderId upsampleShaderId;
	ShaderId applyShaderId;

	unsigned int uniformBlockStride;

	unsigned int uniformBufferId;

	struct UniformBlock
	{
		alignas(8) Vec2f textureScale;
		alignas(4) float thresholdOrIntensity;
		alignas(4) float softThreshold;
	};

public:
	struct RenderParams
	{
		unsigned int sourceTexture;
		unsigned int destinationFramebuffer;

		Vec2i framebufferSize;

		unsigned int iterationCount;
		float bloomThreshold;
		float bloomSoftThreshold;
		float bloomIntensity;
	};

	BloomEffect(
		Allocator* allocator,
		RenderDevice* renderDevice,
		PostProcessRenderer* postProcessRenderer,
		RenderTargetContainer* renderTargetContainer,
		ShaderManager* shaderManager);
	~BloomEffect();

	void Initialize();
	void Deinitialize();

	void Render(const RenderParams& params);
};
