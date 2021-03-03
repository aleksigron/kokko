#pragma once

#include "Core/Array.hpp"

#include "Math/Vec2.hpp"

#include "Resources/ShaderId.hpp"

class Allocator;
class RenderDevice;
class PostProcessRenderer;
class RenderTargetContainer;
class ShaderManager;

class BloomEffect
{
public:
	struct Params
	{
		unsigned int iterationCount;
		float bloomThreshold;
		float bloomSoftThreshold;
		float bloomIntensity;
	};

private:
	Allocator* allocator;
	RenderDevice* renderDevice;
	ShaderManager* shaderManager;
	PostProcessRenderer* postProcessRenderer;

	Array<float> blurKernel;

	unsigned char* uniformStagingBuffer;

	ShaderId extractShaderId;
	ShaderId downsampleShaderId;
	ShaderId upsampleShaderId;
	ShaderId applyShaderId;

	unsigned int uniformBlockStride;

	unsigned int uniformBufferId;
	unsigned int linearSamplerId;

	Params bloomParams;

	void CreateKernel(int kernelExtent);

public:
	BloomEffect(
		Allocator* allocator,
		RenderDevice* renderDevice,
		ShaderManager* shaderManager,
		PostProcessRenderer* postProcessRenderer);

	~BloomEffect();

	void Initialize();
	void Deinitialize();

	void SetParams(const Params& params);
	void Render(
		unsigned int sourceTexture,
		unsigned int destinationFramebuffer,
		const Vec2i& framebufferSize);
};
