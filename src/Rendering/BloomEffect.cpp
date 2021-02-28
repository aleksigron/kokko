#include "Rendering/BloomEffect.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/PostProcessRenderer.hpp"
#include "Rendering/PostProcessRenderPass.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderTargetContainer.hpp"

#include "Resources/ShaderManager.hpp"

BloomEffect::BloomEffect(
	Allocator* allocator,
	RenderDevice* renderDevice,
	PostProcessRenderer* postProcessRenderer,
	RenderTargetContainer* renderTargetContainer,
	ShaderManager* shaderManager) :
	allocator(allocator),
	renderDevice(renderDevice),
	postProcessRenderer(postProcessRenderer),
	renderTargetContainer(renderTargetContainer),
	shaderManager(shaderManager),
	uniformStagingBuffer(nullptr)
{
	extractShaderId = ShaderId{ 0 };
	downsampleShaderId = ShaderId{ 0 };
	upsampleShaderId = ShaderId{ 0 };
	applyShaderId = ShaderId{ 0 };

	uniformBlockStride = 0;

	uniformBufferId = 0;
}

BloomEffect::~BloomEffect()
{
}

void BloomEffect::Initialize()
{
	StringRef extractPath("res/shaders/post_process/bloom_extract.shader.json");
	StringRef downsamplePath("res/shaders/post_process/bloom_downsample.shader.json");
	StringRef upsamplePath("res/shaders/post_process/bloom_upsample.shader.json");
	StringRef applyPath("res/shaders/post_process/bloom_apply.shader.json");

	extractShaderId = shaderManager->GetIdByPath(extractPath);
	downsampleShaderId = shaderManager->GetIdByPath(downsamplePath);
	upsampleShaderId = shaderManager->GetIdByPath(upsamplePath);
	applyShaderId = shaderManager->GetIdByPath(applyPath);

	int aligment = 0;
	renderDevice->GetIntegerValue(RenderDeviceParameter::UniformBufferOffsetAlignment, &aligment);
	uniformBlockStride = (sizeof(UniformBlock) + aligment - 1) / aligment * aligment;

	unsigned int blockCount = 32;
	unsigned int bufferSize = uniformBlockStride * blockCount;

	uniformStagingBuffer = static_cast<unsigned char*>(allocator->Allocate(bufferSize));

	renderDevice->CreateBuffers(1, &uniformBufferId);
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferId);
	renderDevice->SetBufferData(RenderBufferTarget::UniformBuffer, bufferSize, nullptr, RenderBufferUsage::DynamicDraw);
}

void BloomEffect::Deinitialize()
{
	if (uniformStagingBuffer != nullptr)
	{
		allocator->Deallocate(uniformStagingBuffer);
		uniformStagingBuffer = nullptr;
	}

	if (uniformBufferId != 0)
	{
		renderDevice->DestroyBuffers(1, &uniformBufferId);
		uniformBufferId = 0;
	}
}

void BloomEffect::Render(const RenderParams& params)
{
	unsigned int passCount = 0;

	RenderTarget* renderTargets[8];
	PostProcessRenderPass renderPasses[16];

	unsigned int sourceTexture = params.sourceTexture;
	RenderTarget* currentSource = nullptr;
	RenderTarget* currentDestination = nullptr;

	Vec2i size = params.framebufferSize;

	PostProcessRenderPass pass;
	pass.textureNameHashes[0] = "source_map"_hash;
	pass.textureCount = 1;
	pass.uniformBufferId = uniformBufferId;
	pass.uniformBindingPoint = 0;
	pass.uniformBufferRangeSize = sizeof(UniformBlock);
	pass.enableBlending = false;

	int rtIdx = 0;
	for (; rtIdx < params.iterationCount; ++rtIdx)
	{
		size.x /= 2;
		size.y /= 2;

		if (size.x < 2 || size.y < 2)
			break;

		currentDestination = renderTargetContainer->AcquireRenderTarget(size, RenderTextureSizedFormat::RGB16F);
		renderTargets[rtIdx] = currentDestination;

		Vec2i sourceSize = rtIdx == 0 ? params.framebufferSize : currentSource->size;

		UniformBlock* block = reinterpret_cast<UniformBlock*>(&uniformStagingBuffer[uniformBlockStride * passCount]);
		block->textureScale = Vec2f(1.0f / sourceSize.x, 1.0f / sourceSize.y);
		block->thresholdOrIntensity = params.bloomThreshold;
		block->softThreshold = params.bloomSoftThreshold;

		pass.textureIds[0] = rtIdx == 0 ? params.sourceTexture : currentSource->colorTexture;
		pass.uniformBufferRangeStart = uniformBlockStride * passCount;
		pass.framebufferId = currentDestination->framebuffer;
		pass.viewportSize = renderTargets[rtIdx]->size;
		pass.shaderId = rtIdx == 0 ? extractShaderId : downsampleShaderId;

		renderPasses[passCount] = pass;
		passCount += 1;

		currentSource = currentDestination;
	}

	pass.enableBlending = true;
	pass.sourceBlendFactor = RenderBlendFactor::One;
	pass.destinationBlendFactor = RenderBlendFactor::One;

	for (rtIdx -= 2; rtIdx >= 0; rtIdx--)
	{
		UniformBlock* block = reinterpret_cast<UniformBlock*>(&uniformStagingBuffer[uniformBlockStride * passCount]);
		block->textureScale = Vec2f(1.0f / currentSource->size.x, 1.0f / currentSource->size.y);
		block->thresholdOrIntensity = params.bloomThreshold;

		currentDestination = renderTargets[rtIdx];

		pass.textureIds[0] = currentSource->colorTexture;
		pass.uniformBufferRangeStart = uniformBlockStride * passCount;
		pass.framebufferId = currentDestination->framebuffer;
		pass.viewportSize = renderTargets[rtIdx]->size;
		pass.shaderId = upsampleShaderId;

		renderPasses[passCount] = pass;
		passCount += 1;

		currentSource = currentDestination;
	}

	UniformBlock* block = reinterpret_cast<UniformBlock*>(&uniformStagingBuffer[uniformBlockStride * passCount]);
	block->textureScale = Vec2f(1.0f / currentSource->size.x, 1.0f / currentSource->size.y);
	block->thresholdOrIntensity = params.bloomIntensity;
	
	pass.textureIds[0] = currentSource->colorTexture;
	pass.uniformBufferRangeStart = uniformBlockStride * passCount;
	pass.framebufferId = params.destinationFramebuffer;
	pass.viewportSize = params.framebufferSize;
	pass.shaderId = applyShaderId;

	renderPasses[passCount] = pass;
	passCount += 1;

	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferId);
	renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, uniformBlockStride * passCount, uniformStagingBuffer);

	renderDevice->DepthTestDisable();

	for (unsigned int i = 0; i < passCount; ++i)
		postProcessRenderer->RenderPass(renderPasses[i]);

	for (unsigned int i = 0; i < params.iterationCount; ++i)
		renderTargetContainer->ReleaseRenderTarget(renderTargets[i]);
}
