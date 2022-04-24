#include "Graphics/GraphicsFeatureBloom.hpp"

#include "Graphics/GraphicsFeatureCommandList.hpp"

#include "Rendering/RenderCommandType.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderDeviceEnums.hpp"
#include "Rendering/RenderGraphResources.hpp"
#include "Rendering/RenderTargetContainer.hpp"
#include "Rendering/PostProcessRenderer.hpp"
#include "Rendering/PostProcessRenderPass.hpp"
#include "Rendering/StaticUniformBuffer.hpp"

#include "Resources/ShaderManager.hpp"

namespace kokko
{

namespace
{

static const size_t MaxKernelSize = 25;
static const int KernelExtent = 2;

struct ExtractUniforms
{
	alignas(8) Vec2f textureScale;
	alignas(4) float threshold;
	alignas(4) float softThreshold;
};

struct DownsampleUniforms
{
	alignas(16) Vec2f textureScale;
};

struct UpsampleUniforms
{
	UniformBlockArray<float, MaxKernelSize> kernel;

	alignas(8) Vec2f textureScale;
	alignas(4) int kernelExtent;
};

struct ApplyUniforms
{
	UniformBlockArray<float, MaxKernelSize> kernel;

	alignas(8) Vec2f textureScale;
	alignas(4) int kernelExtent;
	alignas(4) float intensity;
};

}// Anonymous namespace

GraphicsFeatureBloom::GraphicsFeatureBloom(Allocator* allocator) :
	allocator(allocator),
	blurKernel(allocator),
	uniformStagingBuffer(nullptr),
	extractShaderId(ShaderId::Null),
	downsampleShaderId(ShaderId::Null),
	upsampleShaderId(ShaderId::Null),
	applyShaderId(ShaderId::Null),
	uniformBlockStride(0),
	uniformBufferId(0),
	linearSamplerId(0)
{
}

void GraphicsFeatureBloom::SetOrder(unsigned int order)
{
	renderOrder = order;
}

void GraphicsFeatureBloom::Initialize(const InitializeParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	RenderDevice* renderDevice = parameters.renderDevice;

	ConstStringView extractPath("engine/shaders/post_process/bloom_extract.glsl");
	ConstStringView downsamplePath("engine/shaders/post_process/bloom_downsample.glsl");
	ConstStringView upsamplePath("engine/shaders/post_process/bloom_upsample.glsl");
	ConstStringView applyPath("engine/shaders/post_process/bloom_apply.glsl");

	extractShaderId = parameters.shaderManager->FindShaderByPath(extractPath);
	downsampleShaderId = parameters.shaderManager->FindShaderByPath(downsamplePath);
	upsampleShaderId = parameters.shaderManager->FindShaderByPath(upsamplePath);
	applyShaderId = parameters.shaderManager->FindShaderByPath(applyPath);

	size_t maxBlockSize = std::max({
		sizeof(ExtractUniforms), sizeof(DownsampleUniforms), sizeof(UpsampleUniforms), sizeof(ApplyUniforms) });

	int aligment = 0;
	renderDevice->GetIntegerValue(RenderDeviceParameter::UniformBufferOffsetAlignment, &aligment);
	uniformBlockStride = static_cast<unsigned int>((maxBlockSize + aligment - 1) / aligment * aligment);

	unsigned int blockCount = 32;
	unsigned int bufferSize = uniformBlockStride * blockCount;

	uniformStagingBuffer = static_cast<unsigned char*>(allocator->Allocate(bufferSize, "GraphicsFeatureBloom::uniformStagingBuffer"));

	renderDevice->CreateBuffers(1, &uniformBufferId);
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferId);

	RenderCommandData::SetBufferStorage storage{};
	storage.target = RenderBufferTarget::UniformBuffer;
	storage.size = bufferSize;
	storage.data = nullptr;
	storage.dynamicStorage = true;
	renderDevice->SetBufferStorage(&storage);

	renderDevice->CreateSamplers(1, &linearSamplerId);

	RenderTextureFilterMode linear = RenderTextureFilterMode::Linear;
	RenderTextureWrapMode clamp = RenderTextureWrapMode::ClampToEdge;

	RenderCommandData::SetSamplerParameters linearSamplerParams{
		linearSamplerId, linear, linear, clamp, clamp, clamp, RenderTextureCompareMode::None
	};
	renderDevice->SetSamplerParameters(&linearSamplerParams);

	CreateKernel(KernelExtent);
}

void GraphicsFeatureBloom::Deinitialize(const InitializeParameters& parameters)
{
	if (linearSamplerId != 0)
	{
		parameters.renderDevice->DestroySamplers(1, &linearSamplerId);
		linearSamplerId = 0;
	}

	if (uniformStagingBuffer != nullptr)
	{
		allocator->Deallocate(uniformStagingBuffer);
		uniformStagingBuffer = nullptr;
	}

	if (uniformBufferId != 0)
	{
		parameters.renderDevice->DestroyBuffers(1, &uniformBufferId);
		uniformBufferId = 0;
	}
}

void GraphicsFeatureBloom::Submit(const SubmitParameters& parameters)
{
	parameters.commandList.AddToFullscreenViewportWithOrder(RenderPass::PostProcess, renderOrder, 0);
}

void GraphicsFeatureBloom::Render(const RenderParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	RenderDevice* renderDevice = parameters.renderDevice;
	PostProcessRenderer* postProcessRenderer = parameters.postProcessRenderer;
	Vec2i framebufferSize = parameters.fullscreenViewport.viewportRectangle.size;

	unsigned int passCount = 0;

	RenderTarget renderTargets[8];
	PostProcessRenderPass renderPasses[16];

	RenderTarget* currentSource = nullptr;
	RenderTarget* currentDestination = nullptr;

	unsigned int extractPassSampler = linearSamplerId;
	unsigned int downsamplePassSampler = linearSamplerId;
	unsigned int upsamplePassSampler = linearSamplerId;
	unsigned int applyPassSampler = linearSamplerId;

	int iterationCount = 4;
	float bloomThreshold = 1.2f;
	float bloomSoftThreshold = 0.8f;
	float bloomIntensity = 0.6f;

	PostProcessRenderPass pass;
	pass.textureNameHashes[0] = "source_map"_hash;
	pass.textureCount = 1;
	pass.uniformBufferId = uniformBufferId;
	pass.uniformBindingPoint = UniformBlockBinding::Object;
	pass.enableBlending = false;

	// EXTRACT PASS

	Vec2i size(framebufferSize.x / 2, framebufferSize.y / 2);

	RenderTargetContainer* renderTargetContainer = postProcessRenderer->GetRenderTargetContainer();
	renderTargets[0] = renderTargetContainer->AcquireRenderTarget(size, RenderTextureSizedFormat::RGB16F);
	currentDestination = &renderTargets[0];

	ExtractUniforms* extractBlock = reinterpret_cast<ExtractUniforms*>(&uniformStagingBuffer[uniformBlockStride * passCount]);
	extractBlock->textureScale = Vec2f(1.0f / framebufferSize.x, 1.0f / framebufferSize.y);
	extractBlock->threshold = bloomThreshold;
	extractBlock->softThreshold = bloomSoftThreshold;

	pass.textureIds[0] = parameters.renderGraphResources->GetLightAccumulationBuffer().GetColorTextureId(0);
	pass.samplerIds[0] = extractPassSampler;
	pass.uniformBufferRangeStart = uniformBlockStride * passCount;
	pass.uniformBufferRangeSize = sizeof(ExtractUniforms);
	pass.framebufferId = currentDestination->framebuffer;
	pass.viewportSize = currentDestination->size;
	pass.shaderId = extractShaderId;

	renderPasses[passCount] = pass;
	passCount += 1;

	currentSource = currentDestination;

	// DOWNSAMPLE PASSES

	int rtIdx = 1;
	for (; rtIdx < iterationCount; ++rtIdx)
	{
		size.x /= 2;
		size.y /= 2;

		if (size.x < 2 || size.y < 2)
			break;

		renderTargets[rtIdx] = renderTargetContainer->AcquireRenderTarget(size, RenderTextureSizedFormat::RGB16F);
		currentDestination = &renderTargets[rtIdx];

		DownsampleUniforms* block = reinterpret_cast<DownsampleUniforms*>(&uniformStagingBuffer[uniformBlockStride * passCount]);
		block->textureScale = Vec2f(1.0f / currentSource->size.x, 1.0f / currentSource->size.y);

		pass.textureIds[0] = currentSource->colorTexture;
		pass.samplerIds[0] = downsamplePassSampler;
		pass.uniformBufferRangeStart = uniformBlockStride * passCount;
		pass.uniformBufferRangeSize = sizeof(DownsampleUniforms);
		pass.framebufferId = currentDestination->framebuffer;
		pass.viewportSize = currentDestination->size;
		pass.shaderId = downsampleShaderId;

		renderPasses[passCount] = pass;
		passCount += 1;

		currentSource = currentDestination;
	}

	// UPSAMPLE PASSES

	pass.enableBlending = true;
	pass.sourceBlendFactor = RenderBlendFactor::One;
	pass.destinationBlendFactor = RenderBlendFactor::One;

	for (rtIdx -= 2; rtIdx >= 0; rtIdx--)
	{
		UpsampleUniforms* block = reinterpret_cast<UpsampleUniforms*>(&uniformStagingBuffer[uniformBlockStride * passCount]);

		for (size_t i = 0; i < MaxKernelSize; ++i)
			block->kernel[i] = blurKernel[i];

		block->textureScale = Vec2f(1.0f / currentSource->size.x, 1.0f / currentSource->size.y);
		block->kernelExtent = KernelExtent;

		currentDestination = &renderTargets[rtIdx];

		pass.textureIds[0] = currentSource->colorTexture;
		pass.samplerIds[0] = upsamplePassSampler;
		pass.uniformBufferRangeStart = uniformBlockStride * passCount;
		pass.uniformBufferRangeSize = sizeof(UpsampleUniforms);
		pass.framebufferId = currentDestination->framebuffer;
		pass.viewportSize = currentDestination->size;
		pass.shaderId = upsampleShaderId;

		renderPasses[passCount] = pass;
		passCount += 1;

		currentSource = currentDestination;
	}

	// APPLY PASS

	ApplyUniforms* applyBlock = reinterpret_cast<ApplyUniforms*>(&uniformStagingBuffer[uniformBlockStride * passCount]);
	for (size_t i = 0; i < MaxKernelSize; ++i)
		applyBlock->kernel[i] = blurKernel[i];

	applyBlock->textureScale = Vec2f(1.0f / currentSource->size.x, 1.0f / currentSource->size.y);
	applyBlock->kernelExtent = KernelExtent;
	applyBlock->intensity = bloomIntensity;

	pass.textureIds[0] = currentSource->colorTexture;
	pass.samplerIds[0] = applyPassSampler;
	pass.uniformBufferRangeStart = uniformBlockStride * passCount;
	pass.uniformBufferRangeSize = sizeof(ApplyUniforms);
	pass.framebufferId = parameters.renderGraphResources->GetLightAccumulationBuffer().GetFramebufferId();
	pass.viewportSize = framebufferSize;
	pass.shaderId = applyShaderId;

	renderPasses[passCount] = pass;
	passCount += 1;

	// Update uniform buffer

	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferId);
	renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, uniformBlockStride * passCount, uniformStagingBuffer);

	// Render passes

	renderDevice->DepthTestDisable();

	postProcessRenderer->RenderPasses(passCount, renderPasses);

	// Release render targets

	for (int i = 0; i < iterationCount; ++i)
		renderTargetContainer->ReleaseRenderTarget(renderTargets[i].id);
}

void GraphicsFeatureBloom::CreateKernel(int kernelExtent)
{
	KOKKO_PROFILE_FUNCTION();

	int kernelWidth = (2 * kernelExtent + 1);
	blurKernel.Resize(kernelWidth * kernelWidth);

	const float sigma = std::max(kernelExtent / 2.0f, 1.0f);
	const float exponentDenominator = (2.0f * sigma * sigma);
	const float valueDenominator = exponentDenominator * Math::Const::Pi;

	float sum = 0.0f;

	for (int y = -kernelExtent; y <= kernelExtent; ++y)
	{
		for (int x = -kernelExtent; x <= kernelExtent; ++x)
		{
			float exponentNumerator = static_cast<float>(-(x * x + y * y));
			float eExpression = std::pow(Math::Const::e, exponentNumerator / exponentDenominator);
			float kernelValue = eExpression / valueDenominator;

			int index = (y + kernelExtent) * kernelWidth + (x + kernelExtent);
			blurKernel[index] = kernelValue;
			sum += kernelValue;
		}
	}

	float multiplier = 1.0f / sum;

	for (unsigned int i = 0, count = kernelWidth * kernelWidth; i < count; ++i)
		blurKernel[i] *= multiplier;
}

}
