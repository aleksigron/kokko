#include "Graphics/GraphicsFeatureSsao.hpp"

#include "Core/Core.hpp"

#include "Math/Math.hpp"
#include "Math/Random.hpp"

#include "Graphics/GraphicsFeatureCommandList.hpp"

#include "Rendering/PostProcessRenderer.hpp"
#include "Rendering/PostProcessRenderPass.hpp"
#include "Rendering/RenderCommandData.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderTypes.hpp"
#include "Rendering/RenderGraphResources.hpp"
#include "Rendering/RenderPassType.hpp"
#include "Rendering/RenderTargetContainer.hpp"
#include "Rendering/StaticUniformBuffer.hpp"

#include "Resources/ShaderManager.hpp"

namespace kokko
{

namespace
{

static constexpr size_t MaxKernelSize = 64;
static constexpr size_t KernelSize = 16;
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

} // Anonymous namespace

GraphicsFeatureSsao::GraphicsFeatureSsao(Allocator* allocator) :
	allocator(allocator),
	kernel(allocator),
	renderOrder(0),
	noiseTextureId(0)
{
	uniformBufferIds[0] = RenderBufferId();
	uniformBufferIds[1] = RenderBufferId();

	shaderIds[0] = ShaderId::Null;
	shaderIds[1] = ShaderId::Null;
}

void GraphicsFeatureSsao::SetOrder(unsigned int order)
{
	renderOrder = order;
}

void GraphicsFeatureSsao::Initialize(const InitializeParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	RenderDevice* device = parameters.renderDevice;

	kernel.Resize(KernelSize);
	for (int i = 0; i < KernelSize;)
	{
		Vec3f vec(Random::Float(-1.0f, 1.0f),
			Random::Float(-1.0f, 1.0f),
			Random::Float01());

		if (vec.SqrMagnitude() <= 1.0f)
		{
			float scale = i / static_cast<float>(KernelSize);
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

	device->CreateTextures(RenderTextureTarget::Texture2d, 1, &noiseTextureId);
	//device->BindTexture(RenderTextureTarget::Texture2d, noiseTextureId);
	//device->SetTextureMinFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);
	//device->SetTextureMagFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);
	//device->SetTextureWrapModeU(RenderTextureTarget::Texture2d, RenderTextureWrapMode::Repeat);
	//device->SetTextureWrapModeV(RenderTextureTarget::Texture2d, RenderTextureWrapMode::Repeat);

	device->SetTextureStorage2D(noiseTextureId, 1, RenderTextureSizedFormat::RG16, NoiseTextureSize, NoiseTextureSize);

	device->SetTextureSubImage2D(noiseTextureId, 0, 0, 0, NoiseTextureSize, NoiseTextureSize,
		RenderTextureBaseFormat::RG, RenderTextureDataType::UnsignedShort, noise.GetData());

	// Per pass resources

	kokko::ConstStringView shaderPaths[PassCount] = {
		kokko::ConstStringView("engine/shaders/post_process/ssao_occlusion.glsl"),
		kokko::ConstStringView("engine/shaders/post_process/ssao_blur.glsl")
	};

	size_t uniformSizes[PassCount] = {
		sizeof(OcclusionUniformBlock),
		sizeof(BlurUniformBlock)
	};

	device->CreateBuffers(static_cast<unsigned int>(PassCount), uniformBufferIds);

	for (unsigned int i = 0; i < PassCount; ++i)
	{
		// Uniform buffer
		device->SetBufferStorage(uniformBufferIds[i], uniformSizes[i], nullptr, BufferStorageFlags::Dynamic);

		// Shader
		shaderIds[i] = parameters.shaderManager->FindShaderByPath(shaderPaths[i]);
	}
}
void GraphicsFeatureSsao::Deinitialize(const InitializeParameters& parameters)
{
}
void GraphicsFeatureSsao::Upload(const UploadParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	RenderDevice* device = parameters.renderDevice;
	Vec2i framebufferSize = parameters.fullscreenViewport.viewportRectangle.size;

	float noiseSizef = static_cast<float>(NoiseTextureSize);

	const ProjectionParameters& projection = parameters.cameraParameters.projection;
	bool reverseDepth = true;
	Mat4x4f projectionMat = parameters.cameraParameters.projection.GetProjectionMatrix(reverseDepth);

	OcclusionUniformBlock occlusionUniforms;
	occlusionUniforms.projection = projectionMat;
	occlusionUniforms.halfNearPlane.y = std::tan(projection.perspectiveFieldOfView * 0.5f);
	occlusionUniforms.halfNearPlane.x = occlusionUniforms.halfNearPlane.y * projection.aspect;
	occlusionUniforms.noiseScale = Vec2f(framebufferSize.x / noiseSizef, framebufferSize.y / noiseSizef);
	occlusionUniforms.sampleRadius = 0.5f;
	occlusionUniforms.kernelSize = KernelSize;

	for (size_t i = 0; i < KernelSize; ++i)
		occlusionUniforms.kernel[i] = kernel[i];

	BlurUniformBlock blurUniforms;
	blurUniforms.textureScale = Vec2f(1.0f / framebufferSize.x, 1.0f / framebufferSize.y);

	device->SetBufferSubData(uniformBufferIds[PassIdx_Occlusion], 0, sizeof(OcclusionUniformBlock), &occlusionUniforms);
	device->SetBufferSubData(uniformBufferIds[PassIdx_Blur], 0, sizeof(BlurUniformBlock), &blurUniforms);
}

void GraphicsFeatureSsao::Submit(const SubmitParameters& parameters)
{
	parameters.commandList.AddToFullscreenViewportWithOrder(RenderPassType::OpaqueLighting, renderOrder, 0);
}

void GraphicsFeatureSsao::Render(const RenderParameters& parameters)
{
	Vec2i framebufferSize = parameters.fullscreenViewport.viewportRectangle.size;

	// Get render targets

	PostProcessRenderer* postProcessRenderer = parameters.postProcessRenderer;
	RenderTargetContainer* renderTargetContainer = postProcessRenderer->GetRenderTargetContainer();

	RenderTextureSizedFormat format = RenderTextureSizedFormat::R8;
	RenderTarget occlusionTarget = renderTargetContainer->AcquireRenderTarget(framebufferSize, format);

	PostProcessRenderPass renderPasses[PassCount];

	// SSAO occlusion pass

	PostProcessRenderPass& occlusionPass = renderPasses[PassIdx_Occlusion];

	occlusionPass.textureNameHashes[0] = "g_normal"_hash;
	occlusionPass.textureNameHashes[1] = "g_depth"_hash;
	occlusionPass.textureNameHashes[2] = "noise_texture"_hash;
	occlusionPass.textureIds[0] = parameters.renderGraphResources->GetGeometryBufferNormalTexture();
	occlusionPass.textureIds[1] = parameters.renderGraphResources->GetGeometryBuffer().GetDepthTextureId();
	occlusionPass.textureIds[2] = noiseTextureId;
	occlusionPass.samplerIds[0] = RenderSamplerId();
	occlusionPass.samplerIds[1] = RenderSamplerId();
	occlusionPass.samplerIds[2] = RenderSamplerId();
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
	blurPass.samplerIds[0] = RenderSamplerId();
	blurPass.textureCount = 1;

	blurPass.uniformBufferId = uniformBufferIds[PassIdx_Blur];
	blurPass.uniformBindingPoint = UniformBlockBinding::Object;
	blurPass.uniformBufferRangeStart = 0;
	blurPass.uniformBufferRangeSize = sizeof(OcclusionUniformBlock);

	blurPass.framebufferId = parameters.renderGraphResources->GetAmbientOcclusionBuffer().GetFramebufferId();
	blurPass.viewportSize = framebufferSize;
	blurPass.shaderId = shaderIds[PassIdx_Blur];
	blurPass.enableBlending = false;

	postProcessRenderer->RenderPasses(PassCount, renderPasses);

	renderTargetContainer->ReleaseRenderTarget(occlusionTarget.id);
}

}
