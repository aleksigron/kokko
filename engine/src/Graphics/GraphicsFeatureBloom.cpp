#include "Graphics/GraphicsFeatureBloom.hpp"

#include "Graphics/GraphicsFeatureCommandList.hpp"

#include "Rendering/CommandEncoder.hpp"
#include "Rendering/PostProcessRenderer.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderTypes.hpp"
#include "Rendering/RenderGraphResources.hpp"
#include "Rendering/RenderPassType.hpp"
#include "Rendering/RenderTargetContainer.hpp"
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
	renderPasses(allocator),
	renderTargets(allocator),
	uniformStagingBuffer(allocator),
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

	kokko::render::Device* renderDevice = parameters.renderDevice;

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
	uniformBlockStride = Math::RoundUpToMultiple(static_cast<int>(maxBlockSize), aligment);

	unsigned int blockCount = 32;
	unsigned int bufferSize = uniformBlockStride * blockCount;

	uniformStagingBuffer.Resize(bufferSize);

	renderDevice->CreateBuffers(1, &uniformBufferId);
	renderDevice->SetBufferStorage(uniformBufferId, bufferSize, nullptr, BufferStorageFlags::Dynamic);

	RenderTextureFilterMode linear = RenderTextureFilterMode::Linear;
	RenderTextureWrapMode clamp = RenderTextureWrapMode::ClampToEdge;
	RenderSamplerParameters linearSamplerParams{
		linear, linear, clamp, clamp, clamp, RenderTextureCompareMode::None
	};
	renderDevice->CreateSamplers(1, &linearSamplerParams, &linearSamplerId);

	CreateKernel(KernelExtent);
}

void GraphicsFeatureBloom::Deinitialize(const InitializeParameters& parameters)
{
	if (linearSamplerId != 0)
	{
		parameters.renderDevice->DestroySamplers(1, &linearSamplerId);
		linearSamplerId = render::SamplerId();
	}

	if (uniformBufferId != 0)
	{
		parameters.renderDevice->DestroyBuffers(1, &uniformBufferId);
		uniformBufferId = render::BufferId();
	}
}

void GraphicsFeatureBloom::Upload(const UploadParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	kokko::render::Device* renderDevice = parameters.renderDevice;
	PostProcessRenderer* postProcessRenderer = parameters.postProcessRenderer;
	Vec2i framebufferSize = parameters.fullscreenViewport.viewportRectangle.size;

	RenderTarget* currentSource = nullptr;
	RenderTarget* currentDestination = nullptr;

	render::SamplerId extractPassSampler = linearSamplerId;
	render::SamplerId downsamplePassSampler = linearSamplerId;
	render::SamplerId upsamplePassSampler = linearSamplerId;
	render::SamplerId applyPassSampler = linearSamplerId;

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
	renderTargets.PushBack(renderTargetContainer->AcquireRenderTarget(size, RenderTextureSizedFormat::RGB16F));
	currentDestination = &renderTargets[0];

	auto extractBlock = reinterpret_cast<ExtractUniforms*>(&uniformStagingBuffer[uniformBlockStride * renderPasses.GetCount()]);
	extractBlock->textureScale = Vec2f(1.0f / framebufferSize.x, 1.0f / framebufferSize.y);
	extractBlock->threshold = bloomThreshold;
	extractBlock->softThreshold = bloomSoftThreshold;

	pass.textureIds[0] = parameters.renderGraphResources->GetLightAccumulationBuffer().GetColorTextureId(0);
	pass.samplerIds[0] = extractPassSampler;
	pass.uniformBufferRangeStart = uniformBlockStride * renderPasses.GetCount();
	pass.uniformBufferRangeSize = sizeof(ExtractUniforms);
	pass.framebufferId = currentDestination->framebuffer;
	pass.viewportSize = currentDestination->size;
	pass.shaderId = extractShaderId;

	renderPasses.PushBack(pass);

	currentSource = currentDestination;

	// DOWNSAMPLE PASSES

	int rtIdx = 1;
	for (; rtIdx < iterationCount; ++rtIdx)
	{
		size.x /= 2;
		size.y /= 2;

		if (size.x < 2 || size.y < 2)
			break;

		renderTargets.PushBack(renderTargetContainer->AcquireRenderTarget(size, RenderTextureSizedFormat::RGB16F));
		currentDestination = &renderTargets[rtIdx];

		DownsampleUniforms* block = 
			reinterpret_cast<DownsampleUniforms*>(&uniformStagingBuffer[uniformBlockStride * renderPasses.GetCount()]);
		block->textureScale = Vec2f(1.0f / currentSource->size.x, 1.0f / currentSource->size.y);

		pass.textureIds[0] = currentSource->colorTexture;
		pass.samplerIds[0] = downsamplePassSampler;
		pass.uniformBufferRangeStart = uniformBlockStride * renderPasses.GetCount();
		pass.uniformBufferRangeSize = sizeof(DownsampleUniforms);
		pass.framebufferId = currentDestination->framebuffer;
		pass.viewportSize = currentDestination->size;
		pass.shaderId = downsampleShaderId;

		renderPasses.PushBack(pass);

		currentSource = currentDestination;
	}

	// UPSAMPLE PASSES

	pass.enableBlending = true;
	pass.sourceBlendFactor = RenderBlendFactor::One;
	pass.destinationBlendFactor = RenderBlendFactor::One;

	for (rtIdx -= 2; rtIdx >= 0; rtIdx--)
	{
		UpsampleUniforms* block = reinterpret_cast<UpsampleUniforms*>(&uniformStagingBuffer[uniformBlockStride * renderPasses.GetCount()]);

		for (size_t i = 0; i < MaxKernelSize; ++i)
			block->kernel[i] = blurKernel[i];

		block->textureScale = Vec2f(1.0f / currentSource->size.x, 1.0f / currentSource->size.y);
		block->kernelExtent = KernelExtent;

		currentDestination = &renderTargets[rtIdx];

		pass.textureIds[0] = currentSource->colorTexture;
		pass.samplerIds[0] = upsamplePassSampler;
		pass.uniformBufferRangeStart = uniformBlockStride * renderPasses.GetCount();
		pass.uniformBufferRangeSize = sizeof(UpsampleUniforms);
		pass.framebufferId = currentDestination->framebuffer;
		pass.viewportSize = currentDestination->size;
		pass.shaderId = upsampleShaderId;

		renderPasses.PushBack(pass);

		currentSource = currentDestination;
	}

	// APPLY PASS

	ApplyUniforms* applyBlock = reinterpret_cast<ApplyUniforms*>(&uniformStagingBuffer[uniformBlockStride * renderPasses.GetCount()]);
	for (size_t i = 0; i < MaxKernelSize; ++i)
		applyBlock->kernel[i] = blurKernel[i];

	applyBlock->textureScale = Vec2f(1.0f / currentSource->size.x, 1.0f / currentSource->size.y);
	applyBlock->kernelExtent = KernelExtent;
	applyBlock->intensity = bloomIntensity;

	pass.textureIds[0] = currentSource->colorTexture;
	pass.samplerIds[0] = applyPassSampler;
	pass.uniformBufferRangeStart = uniformBlockStride * renderPasses.GetCount();
	pass.uniformBufferRangeSize = sizeof(ApplyUniforms);
	pass.framebufferId = parameters.renderGraphResources->GetLightAccumulationBuffer().GetFramebufferId();
	pass.viewportSize = framebufferSize;
	pass.shaderId = applyShaderId;

	renderPasses.PushBack(pass);

	// Update uniform buffer
	renderDevice->SetBufferSubData(uniformBufferId, 0, uniformBlockStride * renderPasses.GetCount(), uniformStagingBuffer.GetData());
}

void GraphicsFeatureBloom::Submit(const SubmitParameters& parameters)
{
	parameters.commandList.AddToFullscreenViewportWithOrder(RenderPassType::PostProcess, renderOrder, 0);
}

void GraphicsFeatureBloom::Render(const RenderParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	PostProcessRenderer* postProcessRenderer = parameters.postProcessRenderer;
	RenderTargetContainer* renderTargetContainer = postProcessRenderer->GetRenderTargetContainer();
	
	parameters.encoder->DepthTestDisable();

	postProcessRenderer->RenderPasses(renderPasses.GetCount(), renderPasses.GetData());

	// Release render targets

	for (const auto& renderTarget : renderTargets)
		renderTargetContainer->ReleaseRenderTarget(renderTarget.id);

	renderPasses.Clear();
	renderTargets.Clear();
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
