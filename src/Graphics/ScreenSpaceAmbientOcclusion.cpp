#include "Graphics/ScreenSpaceAmbientOcclusion.hpp"

#include "Core/Core.hpp"

#include "Math/Math.hpp"
#include "Math/Projection.hpp"
#include "Math/Random.hpp"

#include "Rendering/PostProcessRenderer.hpp"
#include "Rendering/PostProcessRenderPass.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderTargetContainer.hpp"
#include "Rendering/RenderViewport.hpp"

#include "Resources/ShaderManager.hpp"

ScreenSpaceAmbientOcclusion::ScreenSpaceAmbientOcclusion(
	Allocator* allocator,
	RenderDevice* renderDevice,
	ShaderManager* shaderManager,
	PostProcessRenderer* postProcessRenderer) :
	allocator(allocator),
	renderDevice(renderDevice),
	shaderManager(shaderManager),
	postProcessRenderer(postProcessRenderer),
	kernel(allocator)
{
	resultRenderTargetId = 0;
	resultTextureId = 0;

	kernelSize = 16;

	for (unsigned int i = 0; i < PassCount; ++i)
	{
		uniformBufferIds[i] = 0;
		shaderIds[i] = ShaderId{ 0 };
	}

	noiseTextureId = 0;
}

ScreenSpaceAmbientOcclusion::~ScreenSpaceAmbientOcclusion()
{
	Deinitialize();
}

void ScreenSpaceAmbientOcclusion::Initialize(Vec2i framebufferResolution)
{
	KOKKO_PROFILE_FUNCTION();

	framebufferSize = framebufferResolution;

	kernel.Resize(kernelSize);
	for (unsigned int i = 0; i < kernelSize;)
	{
		Vec3f vec(Random::Float(-1.0f, 1.0f),
			Random::Float(-1.0f, 1.0f),
			Random::Float01());

		if (vec.SqrMagnitude() <= 1.0f)
		{
			float scale = i / static_cast<float>(kernelSize);
			scale = Math::Lerp(0.1f, 1.0f, scale * scale);
			kernel[i] = vec.GetNormalized() * scale;
			i += 1;
		}
	}

	// Noise texture

	const unsigned int noisePixels = NoiseTextureSize * NoiseTextureSize;

	Array<uint16_t> noise(allocator);
	noise.Resize(noisePixels * 2);
	for (unsigned int i = 0; i < noisePixels; i++)
	{
		noise[i * 2 + 0] = static_cast<uint16_t>(Random::Uint(0, UINT16_MAX));
		noise[i * 2 + 1] = static_cast<uint16_t>(Random::Uint(0, UINT16_MAX));
	}

	renderDevice->CreateTextures(1, &noiseTextureId);
	renderDevice->BindTexture(RenderTextureTarget::Texture2d, noiseTextureId);
	renderDevice->SetTextureMinFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);
	renderDevice->SetTextureMagFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);
	renderDevice->SetTextureWrapModeU(RenderTextureTarget::Texture2d, RenderTextureWrapMode::Repeat);
	renderDevice->SetTextureWrapModeV(RenderTextureTarget::Texture2d, RenderTextureWrapMode::Repeat);

	RenderCommandData::SetTextureStorage2D noiseStorage{
		RenderTextureTarget::Texture2d, 1, RenderTextureSizedFormat::RG16, NoiseTextureSize, NoiseTextureSize
	};
	renderDevice->SetTextureStorage2D(&noiseStorage);

	RenderCommandData::SetTextureSubImage2D noiseImage{
		RenderTextureTarget::Texture2d, 0, 0, 0, NoiseTextureSize, NoiseTextureSize,
		RenderTextureBaseFormat::RG, RenderTextureDataType::UnsignedShort, noise.GetData()
	};
	renderDevice->SetTextureSubImage2D(&noiseImage);

	// Per pass resources

	StringRef shaderPaths[] = {
		StringRef("res/shaders/post_process/ssao_occlusion.shader.json"),
		StringRef("res/shaders/post_process/ssao_blur.shader.json")
	};

	size_t uniformSizes[] = {
		sizeof(OcclusionUniformBlock),
		sizeof(BlurUniformBlock)
	};

	CreatePassResources(PassCount, shaderPaths, uniformSizes);
}

void ScreenSpaceAmbientOcclusion::Deinitialize()
{
	if (uniformBufferIds[0] != 0)
	{
		renderDevice->DestroyBuffers(PassCount, uniformBufferIds);
		for (unsigned int i = 0; i < PassCount; ++i)
			uniformBufferIds[i] = 0;
	}

	if (noiseTextureId != 0)
	{
		renderDevice->DestroyTextures(1, &noiseTextureId);
		noiseTextureId = 0;
	}
}

void ScreenSpaceAmbientOcclusion::Render(const RenderParams& params)
{
	KOKKO_PROFILE_FUNCTION();

	// Update uniforms

	UpdateUniformBuffers(params.projection);

	// Get render targets

	RenderTargetContainer* renderTargetContainer = postProcessRenderer->GetRenderTargetContainer();

	RenderTextureSizedFormat format = RenderTextureSizedFormat::R8;
	RenderTarget occlusionTarget = renderTargetContainer->AcquireRenderTarget(framebufferSize, format);
	RenderTarget blurTarget = renderTargetContainer->AcquireRenderTarget(framebufferSize, format);

	PostProcessRenderPass renderPasses[PassCount];

	// SSAO occlusion pass

	PostProcessRenderPass& occlusionPass = renderPasses[PassIdx_Occlusion];

	occlusionPass.textureNameHashes[0] = "g_normal"_hash;
	occlusionPass.textureNameHashes[1] = "g_depth"_hash;
	occlusionPass.textureNameHashes[2] = "noise_texture"_hash;
	occlusionPass.textureIds[0] = params.normalTexture;
	occlusionPass.textureIds[1] = params.depthTexture;
	occlusionPass.textureIds[2] = noiseTextureId;
	occlusionPass.samplerIds[0] = 0;
	occlusionPass.samplerIds[1] = 0;
	occlusionPass.samplerIds[2] = 0;
	occlusionPass.textureCount = 3;

	occlusionPass.uniformBufferId = uniformBufferIds[PassIdx_Occlusion];
	occlusionPass.uniformBindingPoint = UniformBlockBinding::Object;
	occlusionPass.uniformBufferRangeStart = 0;
	occlusionPass.uniformBufferRangeSize = sizeof(OcclusionUniformBlock);

	occlusionPass.framebufferId = occlusionTarget.framebuffer;
	occlusionPass.viewportSize = framebufferSize;
	occlusionPass.shaderId = shaderIds[PassIdx_Occlusion];
	occlusionPass.enableBlending = false;

	// SSAO blur pass

	PostProcessRenderPass& blurPass = renderPasses[PassIdx_Blur];

	blurPass.textureNameHashes[0] = "occlusion_map"_hash;
	blurPass.textureIds[0] = occlusionTarget.colorTexture;
	blurPass.samplerIds[0] = 0;
	blurPass.textureCount = 1;

	blurPass.uniformBufferId = uniformBufferIds[PassIdx_Blur];
	blurPass.uniformBindingPoint = UniformBlockBinding::Object;
	blurPass.uniformBufferRangeStart = 0;
	blurPass.uniformBufferRangeSize = sizeof(OcclusionUniformBlock);

	blurPass.framebufferId = blurTarget.framebuffer;
	blurPass.viewportSize = framebufferSize;
	blurPass.shaderId = shaderIds[PassIdx_Blur];
	blurPass.enableBlending = false;

	postProcessRenderer->RenderPasses(PassCount, renderPasses);

	renderTargetContainer->ReleaseRenderTarget(occlusionTarget.id);

	resultRenderTargetId = blurTarget.id;
	resultTextureId = blurTarget.colorTexture;
}

unsigned int ScreenSpaceAmbientOcclusion::GetResultTextureId()
{
	return resultTextureId;
}

void ScreenSpaceAmbientOcclusion::ReleaseResult()
{
	RenderTargetContainer* renderTargetContainer = postProcessRenderer->GetRenderTargetContainer();
	renderTargetContainer->ReleaseRenderTarget(resultRenderTargetId);

	resultRenderTargetId = 0;
	resultTextureId = 0;
}

void ScreenSpaceAmbientOcclusion::UpdateUniformBuffers(const ProjectionParameters& projection) const
{
	KOKKO_PROFILE_FUNCTION();

	float noiseSizef = static_cast<float>(NoiseTextureSize);

	bool reverseDepth = true;
	Mat4x4f projectionMat = projection.GetProjectionMatrix(reverseDepth);

	OcclusionUniformBlock occlusionUniforms;
	occlusionUniforms.projection = projectionMat;
	occlusionUniforms.halfNearPlane.y = std::tan(projection.perspectiveFieldOfView * 0.5f);
	occlusionUniforms.halfNearPlane.x = occlusionUniforms.halfNearPlane.y * projection.aspect;
	occlusionUniforms.noiseScale = Vec2f(framebufferSize.x / noiseSizef, framebufferSize.y / noiseSizef);
	occlusionUniforms.sampleRadius = 0.5f;
	occlusionUniforms.kernelSize = kernelSize;

	for (size_t i = 0; i < kernelSize; ++i)
		occlusionUniforms.kernel[i] = kernel[i];

	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferIds[PassIdx_Occlusion]);
	renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(OcclusionUniformBlock), &occlusionUniforms);

	BlurUniformBlock blurUniforms;
	blurUniforms.textureScale = Vec2f(1.0f / framebufferSize.x, 1.0f / framebufferSize.y);

	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferIds[PassIdx_Blur]);
	renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(BlurUniformBlock), &blurUniforms);
}

void ScreenSpaceAmbientOcclusion::CreatePassResources(size_t passCount, const StringRef* shaderPaths, const size_t* uniformSizes)
{
	renderDevice->CreateBuffers(passCount, uniformBufferIds);

	for (unsigned int i = 0; i < passCount; ++i)
	{
		// Uniform buffer

		renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferIds[i]);

		RenderCommandData::SetBufferStorage storage{};
		storage.target = RenderBufferTarget::UniformBuffer;
		storage.size = uniformSizes[i];
		storage.data = nullptr;
		storage.dynamicStorage = true;
		renderDevice->SetBufferStorage(&storage);

		// Shader

		shaderIds[i] = shaderManager->GetIdByPath(shaderPaths[i]);
	}
}
