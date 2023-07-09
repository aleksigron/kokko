#include "Graphics/GraphicsFeatureDeferredLighting.hpp"

#include "Core/Core.hpp"

#include "Graphics/EnvironmentSystem.hpp"
#include "Graphics/GraphicsFeatureCommandList.hpp"

#include "Rendering/CascadedShadowMap.hpp"
#include "Rendering/CommandEncoder.hpp"
#include "Rendering/Framebuffer.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/PostProcessRenderer.hpp"
#include "Rendering/PostProcessRenderPass.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderGraphResources.hpp"
#include "Rendering/RenderOrder.hpp"
#include "Rendering/RenderPassType.hpp"
#include "Rendering/StaticUniformBuffer.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"

namespace kokko
{

namespace
{

constexpr int BrdfLutSize = 512;

struct LightingUniformBlock
{
	static constexpr size_t MaxLightCount = 8;
	static constexpr size_t MaxCascadeCount = 4;

	UniformBlockArray<Vec3f, MaxLightCount> lightColors;
	UniformBlockArray<Vec4f, MaxLightCount> lightPositions; // xyz: position, w: inverse square radius
	UniformBlockArray<Vec4f, MaxLightCount> lightDirections; // xyz: direction, w: spot light angle
	UniformBlockArray<bool, MaxLightCount> lightCastShadow;

	UniformBlockArray<Mat4x4f, MaxCascadeCount> shadowMatrices;
	UniformBlockArray<float, MaxCascadeCount + 1> shadowSplits;

	alignas(16) Mat4x4f perspectiveMatrix;
	alignas(16) Mat4x4f viewToWorld;
	alignas(8) Vec2f halfNearPlane;
	alignas(8) Vec2f shadowMapScale;
	alignas(8) Vec2f frameResolution;

	alignas(4) int directionalLightCount;
	alignas(4) int pointLightCount;
	alignas(4) int spotLightCount;
	alignas(4) int cascadeCount;

	alignas(4) float shadowBiasOffset;
	alignas(4) float shadowBiasFactor;
	alignas(4) float shadowBiasClamp;
	alignas(4) float irradianceIntensity;
};

} // Anonymous namespace

GraphicsFeatureDeferredLighting::GraphicsFeatureDeferredLighting(Allocator* allocator) :
	lightResultArray(allocator),
	shaderId(ShaderId::Null),
	meshId(MeshId::Null),
	renderOrder(0),
	uniformBufferId(0),
	brdfLutTextureId(0)
{
	samplers[0] = render::SamplerId();
	samplers[1] = render::SamplerId();
}

void GraphicsFeatureDeferredLighting::SetOrder(unsigned int order)
{
	renderOrder = order;
}

void GraphicsFeatureDeferredLighting::Initialize(const InitializeParameters& parameters)
{
	kokko::render::Device* renderDevice = parameters.renderDevice;

	renderDevice->CreateBuffers(1, &uniformBufferId);
	renderDevice->SetBufferStorage(uniformBufferId, sizeof(LightingUniformBlock), nullptr, BufferStorageFlags::Dynamic);

	ConstStringView label("Renderer deferred lighting uniform buffer");
	renderDevice->SetObjectLabel(RenderObjectType::Buffer, uniformBufferId.i, label);

	ConstStringView shaderPath("engine/shaders/deferred_lighting/lighting.glsl");
	shaderId = parameters.shaderManager->FindShaderByPath(shaderPath);

	// Create screen filling quad
	meshId = parameters.meshManager->CreateMesh();
	MeshPresets::UploadPlane(parameters.meshManager, meshId);
}

void GraphicsFeatureDeferredLighting::Deinitialize(const InitializeParameters& parameters)
{
	if (uniformBufferId != 0)
	{
		parameters.renderDevice->DestroyBuffers(1, &uniformBufferId);
		uniformBufferId = render::BufferId();
	}

	if (brdfLutTextureId != 0)
	{
		parameters.renderDevice->DestroyTextures(1, &brdfLutTextureId);
		brdfLutTextureId = render::TextureId();
	}

	if (brdfLutFramebufferId != 0)
	{
		parameters.renderDevice->DestroyFramebuffers(1, &brdfLutFramebufferId);
		brdfLutFramebufferId = render::FramebufferId();
	}

	if (samplers[0] != 0)
	{
		parameters.renderDevice->DestroySamplers(KOKKO_ARRAY_ITEMS(samplers), samplers);
		samplers[0] = render::SamplerId();
		samplers[1] = render::SamplerId();
		samplers[2] = render::SamplerId();
	}

	if (meshId != MeshId::Null)
	{
		parameters.meshManager->RemoveMesh(meshId);
		meshId = MeshId::Null;
	}
}

void GraphicsFeatureDeferredLighting::Upload(const UploadParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	kokko::render::Device* renderDevice = parameters.renderDevice;

	if (brdfLutFramebufferId != 0) // Delete once the LUT has been rendered
	{
		renderDevice->DestroyFramebuffers(1, &brdfLutFramebufferId);
		brdfLutFramebufferId = render::FramebufferId();
	}

	if (brdfLutTextureId == 0)
	{
		renderDevice->CreateTextures(RenderTextureTarget::Texture2d, 1, &brdfLutTextureId);
		renderDevice->SetTextureStorage2D(brdfLutTextureId, 1, RenderTextureSizedFormat::RG16F, BrdfLutSize, BrdfLutSize);

		kokko::ConstStringView label("Renderer BRDF LUT");
		renderDevice->SetObjectLabel(RenderObjectType::Texture, brdfLutTextureId.i, label);

		renderDevice->CreateFramebuffers(1, &brdfLutFramebufferId);
		renderDevice->AttachFramebufferTexture(brdfLutFramebufferId, RenderFramebufferAttachment::Color0, brdfLutTextureId, 0);
	}

	if (samplers[0] == 0)
	{
		RenderSamplerParameters params[Sampler_COUNT] = {
		{ // Sampler_DepthCompare
			RenderTextureFilterMode::Linear,
			RenderTextureFilterMode::Linear,
			RenderTextureWrapMode::ClampToEdge,
			RenderTextureWrapMode::ClampToEdge,
			RenderTextureWrapMode::ClampToEdge,
			RenderTextureCompareMode::CompareRefToTexture,
			RenderDepthCompareFunc::GreaterThanOrEqual
		},
		{ // Sampler_ClampLinear
			RenderTextureFilterMode::Linear,
			RenderTextureFilterMode::Linear,
			RenderTextureWrapMode::ClampToEdge,
			RenderTextureWrapMode::ClampToEdge,
			RenderTextureWrapMode::ClampToEdge,
			RenderTextureCompareMode::None,
			RenderDepthCompareFunc::Always
		},
		{ // Sampler_Mipmap
			RenderTextureFilterMode::LinearMipmap,
			RenderTextureFilterMode::Linear,
			RenderTextureWrapMode::ClampToEdge,
			RenderTextureWrapMode::ClampToEdge,
			RenderTextureWrapMode::ClampToEdge,
			RenderTextureCompareMode::None,
			RenderDepthCompareFunc::Always
		}};

		renderDevice->CreateSamplers(KOKKO_ARRAY_ITEMS(params), params, samplers);
	}

	LightManager* lightManager = parameters.lightManager;
	EnvironmentSystem* envSystem = parameters.environmentSystem;

	// Both SSAO and deferred lighting passes use these

	ProjectionParameters projParams = parameters.cameraParameters.projection;

	{
		LightingUniformBlock lightingUniforms;

		const RenderViewport& fsvp = parameters.fullscreenViewport;

		// Update directional light viewports
		Array<LightId>& directionalLights = lightResultArray;
		lightManager->GetDirectionalLights(directionalLights);

		Vec2f halfNearPlane;
		halfNearPlane.y = std::tan(projParams.perspectiveFieldOfView * 0.5f);
		halfNearPlane.x = halfNearPlane.y * projParams.aspect;
		lightingUniforms.halfNearPlane = halfNearPlane;

		int shadowSide = CascadedShadowMap::GetShadowCascadeResolution();
		unsigned int cascadeCount = CascadedShadowMap::GetCascadeCount();
		lightingUniforms.shadowMapScale = Vec2f(1.0f / (cascadeCount * shadowSide), 1.0f / shadowSide);

		lightingUniforms.frameResolution = fsvp.viewportRectangle.size.As<float>();

		// Set viewport transform matrices
		lightingUniforms.perspectiveMatrix = fsvp.projection;
		lightingUniforms.viewToWorld = fsvp.view.forward;

		size_t dirLightCount = directionalLights.GetCount();
		lightingUniforms.directionalLightCount = static_cast<int>(dirLightCount);

		// Directional light
		for (size_t i = 0; i < dirLightCount; ++i)
		{
			LightId dirLightId = directionalLights[i];

			Mat3x3f orientation = lightManager->GetOrientation(dirLightId);
			Vec3f wLightDir = orientation * Vec3f(0.0f, 0.0f, -1.0f);
			Vec4f vLightDir = fsvp.view.inverse * Vec4f(wLightDir, 0.0f);
			lightingUniforms.lightDirections[i] = vLightDir;

			lightingUniforms.lightColors[i] = lightManager->GetLightEnergy(dirLightId);
			lightingUniforms.lightCastShadow[i] = lightManager->GetShadowCasting(dirLightId);
		}

		lightResultArray.Clear();
		Array<LightId>& nonDirLights = lightResultArray;
		lightManager->GetNonDirectionalLightsWithinFrustum(fsvp.frustum, nonDirLights);

		// Count the different light types

		unsigned int pointLightCount = 0;
		unsigned int spotLightCount = 0;

		for (size_t lightIdx = 0, count = nonDirLights.GetCount(); lightIdx < count; ++lightIdx)
		{
			LightType type = lightManager->GetLightType(nonDirLights[lightIdx]);
			if (type == LightType::Point)
				pointLightCount += 1;
			else if (type == LightType::Spot)
				spotLightCount += 1;
		}

		lightingUniforms.pointLightCount = pointLightCount;
		lightingUniforms.spotLightCount = spotLightCount;

		size_t pointLightsAdded = 0;
		size_t spotLightsAdded = 0;

		// Light other visible lights
		for (size_t lightIdx = 0, count = nonDirLights.GetCount(); lightIdx < count; ++lightIdx)
		{
			size_t shaderLightIdx;

			LightId lightId = nonDirLights[lightIdx];

			// Point lights come first, so offset spot lights with the amount of point lights
			LightType type = lightManager->GetLightType(lightId);
			if (type == LightType::Spot)
			{
				shaderLightIdx = dirLightCount + pointLightCount + spotLightsAdded;
				spotLightsAdded += 1;

				Mat3x3f orientation = lightManager->GetOrientation(lightId);
				Vec3f wLightDir = orientation * Vec3f(0.0f, 0.0f, -1.0f);
				Vec4f vLightDir = fsvp.view.inverse * Vec4f(wLightDir, 0.0f);
				vLightDir.w = lightManager->GetSpotAngle(lightId);
				lightingUniforms.lightDirections[shaderLightIdx] = vLightDir;
			}
			else
			{
				shaderLightIdx = dirLightCount + pointLightsAdded;
				pointLightsAdded += 1;
			}

			Vec3f wLightPos = lightManager->GetPosition(lightId);
			Vec3f vLightPos = (fsvp.view.inverse * Vec4f(wLightPos, 1.0f)).xyz();

			float radius = lightManager->GetRadius(lightId);
			float inverseSquareRadius = 1.0f / (radius * radius);
			lightingUniforms.lightPositions[shaderLightIdx] = Vec4f(vLightPos, inverseSquareRadius);

			Vec3f lightCol = lightManager->GetLightEnergy(lightId);
			lightingUniforms.lightColors[shaderLightIdx] = lightCol;
			lightingUniforms.lightCastShadow[shaderLightIdx] = false;
		}

		lightResultArray.Clear();

		// shadow_params.splits[0] is the near depth
		lightingUniforms.shadowSplits[0] = projParams.perspectiveNear;

		unsigned int shadowCascadeCount = CascadedShadowMap::GetCascadeCount();
		lightingUniforms.cascadeCount = shadowCascadeCount;

		float cascadeSplitDepths[CascadedShadowMap::MaxCascadeCount];
		CascadedShadowMap::CalculateSplitDepths(projParams, cascadeSplitDepths);

		Mat4x4f bias;
		bias[0] = 0.5f;
		bias[5] = 0.5f;
		bias[12] = 0.5f;
		bias[13] = 0.5f;

		// Update transforms and split depths for each shadow cascade
		for (size_t vpIdx = 0, vpCount = parameters.shadowViewports.GetCount(); vpIdx != vpCount; ++vpIdx)
		{
			Mat4x4f viewToLight = parameters.shadowViewports[vpIdx].viewProjection * fsvp.view.forward;
			Mat4x4f shadowMat = bias * viewToLight;

			lightingUniforms.shadowMatrices[vpIdx] = shadowMat;
			lightingUniforms.shadowSplits[vpIdx + 1] = cascadeSplitDepths[vpIdx];
		}

		lightingUniforms.shadowBiasOffset = 0.001f;
		lightingUniforms.shadowBiasFactor = 0.0019f;
		lightingUniforms.shadowBiasClamp = 0.01f;

		lightingUniforms.irradianceIntensity = 1.0f;
		EnvironmentId envId = envSystem->FindActiveEnvironment();
		if (envId != EnvironmentId::Null)
			lightingUniforms.irradianceIntensity = envSystem->GetIntensity(envId);

		renderDevice->SetBufferSubData(uniformBufferId, 0, sizeof(LightingUniformBlock), &lightingUniforms);
	}
}

void GraphicsFeatureDeferredLighting::Submit(const SubmitParameters& parameters)
{
	if (brdfLutFramebufferId != 0)
	{
		parameters.commandList.AddToStartOfFrame(RenderOrderConfiguration::MaxFeatureObjectId);
	}

	parameters.commandList.AddToFullscreenViewportWithOrder(RenderPassType::OpaqueLighting, renderOrder, 0);
}

void GraphicsFeatureDeferredLighting::Render(const RenderParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	render::CommandEncoder* encoder = parameters.encoder;

	encoder->ScissorTestDisable();
	encoder->DepthTestDisable();
	encoder->BlendingDisable();

	if (parameters.featureObjectId == RenderOrderConfiguration::MaxFeatureObjectId)
	{
		KOKKO_PROFILE_SCOPE("Calculate BRDF LUT");

		// Calculate the BRDF LUT

		encoder->BindFramebuffer(brdfLutFramebufferId);
		encoder->SetViewport(0, 0, BrdfLutSize, BrdfLutSize);

		ConstStringView path("engine/shaders/preprocess/calc_brdf_lut.glsl");
		ShaderId calcBrdfShaderId = parameters.shaderManager->FindShaderByPath(path);
		const ShaderData& calcBrdfShader = parameters.shaderManager->GetShaderData(calcBrdfShaderId);
		encoder->UseShaderProgram(calcBrdfShader.driverId);

		const MeshDrawData* meshDraw = parameters.meshManager->GetDrawData(meshId);
		encoder->BindVertexArray(meshDraw->vertexArrayObject);
		encoder->DrawIndexed(meshDraw->primitiveMode, meshDraw->indexType, meshDraw->count, 0, 0);

		return;
	}

	const RenderGraphResources* resources = parameters.renderGraphResources;

	kokko::EnvironmentTextures envMap;
	kokko::EnvironmentId envId = parameters.environmentSystem->FindActiveEnvironment();
	if (envId != kokko::EnvironmentId::Null)
		envMap = parameters.environmentSystem->GetEnvironmentMap(envId);
	else
		envMap = parameters.environmentSystem->GetEmptyEnvironmentMap();

	const TextureData& diffIrrTexture = parameters.textureManager->GetTextureData(envMap.diffuseIrradianceTexture);
	const TextureData& specIrrTexture = parameters.textureManager->GetTextureData(envMap.specularIrradianceTexture);

	// Deferred lighting

	PostProcessRenderPass deferredPass;

	deferredPass.textureNameHashes[0] = "g_albedo"_hash;
	deferredPass.textureNameHashes[1] = "g_normal"_hash;
	deferredPass.textureNameHashes[2] = "g_material"_hash;
	deferredPass.textureNameHashes[3] = "g_depth"_hash;
	deferredPass.textureNameHashes[4] = "ssao_map"_hash;
	deferredPass.textureNameHashes[5] = "shadow_map"_hash;
	deferredPass.textureNameHashes[6] = "diff_irradiance_map"_hash;
	deferredPass.textureNameHashes[7] = "spec_irradiance_map"_hash;
	deferredPass.textureNameHashes[8] = "brdf_lut"_hash;

	deferredPass.textureIds[0] = resources->GetGeometryBufferAlbedoTexture();
	deferredPass.textureIds[1] = resources->GetGeometryBufferNormalTexture();
	deferredPass.textureIds[2] = resources->GetGeometryBufferMaterialTexture();
	deferredPass.textureIds[3] = resources->GetGeometryBuffer().GetDepthTextureId();
	deferredPass.textureIds[4] = resources->GetAmbientOcclusionBuffer().GetColorTextureId(0);
	deferredPass.textureIds[5] = resources->GetShadowBuffer().GetDepthTextureId();
	deferredPass.textureIds[6] = diffIrrTexture.textureObjectId;
	deferredPass.textureIds[7] = specIrrTexture.textureObjectId;
	deferredPass.textureIds[8] = brdfLutTextureId;

	deferredPass.samplerIds[0] = render::SamplerId();
	deferredPass.samplerIds[1] = render::SamplerId();
	deferredPass.samplerIds[2] = render::SamplerId();
	deferredPass.samplerIds[3] = render::SamplerId();
	deferredPass.samplerIds[4] = render::SamplerId();
	deferredPass.samplerIds[5] = samplers[Sampler_DepthCompare];
	deferredPass.samplerIds[6] = render::SamplerId();
	deferredPass.samplerIds[7] = samplers[Sampler_Mipmap];
	deferredPass.samplerIds[8] = samplers[Sampler_ClampLinear];

	deferredPass.textureCount = 9;

	deferredPass.uniformBufferId = uniformBufferId;
	deferredPass.uniformBindingPoint = UniformBlockBinding::Object;
	deferredPass.uniformBufferRangeStart = 0;
	deferredPass.uniformBufferRangeSize = sizeof(LightingUniformBlock);

	deferredPass.framebufferId = resources->GetLightAccumulationBuffer().GetFramebufferId();
	deferredPass.viewportSize = resources->GetLightAccumulationBuffer().GetSize();
	deferredPass.shaderId = shaderId;
	deferredPass.enableBlending = false;

	parameters.postProcessRenderer->RenderPass(deferredPass);
}

}
