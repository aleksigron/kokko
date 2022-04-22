#include "Graphics/GraphicsFeatureDeferredLighting.hpp"

#include "Core/Core.hpp"

#include "Graphics/EnvironmentSystem.hpp"
#include "Graphics/GraphicsFeatureCommandList.hpp"

#include "Rendering/CascadedShadowMap.hpp"
#include "Rendering/Framebuffer.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/PostProcessRenderer.hpp"
#include "Rendering/PostProcessRenderPass.hpp"
#include "Rendering/RenderCommandType.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderGraphResources.hpp"
#include "Rendering/StaticUniformBuffer.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"

namespace kokko
{

namespace
{

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
}

void GraphicsFeatureDeferredLighting::SetOrder(unsigned int order)
{
	renderOrder = order;
}

void GraphicsFeatureDeferredLighting::Initialize(const InitializeParameters& parameters)
{
	RenderDevice* device = parameters.renderDevice;

	device->CreateBuffers(1, &uniformBufferId);
	device->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferId);

	RenderCommandData::SetBufferStorage storage{};
	storage.target = RenderBufferTarget::UniformBuffer;
	storage.size = sizeof(LightingUniformBlock);
	storage.data = nullptr;
	storage.dynamicStorage = true;
	device->SetBufferStorage(&storage);

	ConstStringView label("Renderer deferred lighting uniform buffer");
	device->SetObjectLabel(RenderObjectType::Buffer, uniformBufferId, label);

	ConstStringView shaderPath("engine/shaders/deferred_lighting/lighting.glsl");
	shaderId = parameters.shaderManager->FindShaderByPath(shaderPath);

	// Create screen filling quad
	meshId = parameters.meshManager->CreateMesh();
	MeshPresets::UploadPlane(parameters.meshManager, meshId);

	{
		KOKKO_PROFILE_SCOPE("Calculate BRDF LUT");

		// Calculate the BRDF LUT

		static const int LutSize = 512;

		device->CreateTextures(1, &brdfLutTextureId);
		device->BindTexture(RenderTextureTarget::Texture2d, brdfLutTextureId);

		RenderCommandData::SetTextureStorage2D storage{
			RenderTextureTarget::Texture2d, 1, RenderTextureSizedFormat::RG16F, LutSize, LutSize
		};
		device->SetTextureStorage2D(&storage);

		device->SetTextureWrapModeU(RenderTextureTarget::Texture2d, RenderTextureWrapMode::ClampToEdge);
		device->SetTextureWrapModeV(RenderTextureTarget::Texture2d, RenderTextureWrapMode::ClampToEdge);
		device->SetTextureMinFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Linear);
		device->SetTextureMagFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Linear);

		unsigned int framebuffer;
		device->CreateFramebuffers(1, &framebuffer);

		RenderCommandData::BindFramebufferData bindFramebuffer{ RenderFramebufferTarget::Framebuffer, framebuffer };
		device->BindFramebuffer(&bindFramebuffer);

		RenderCommandData::AttachFramebufferTexture2D attachTexture{
			RenderFramebufferTarget::Framebuffer, RenderFramebufferAttachment::Color0,
			RenderTextureTarget::Texture2d, brdfLutTextureId, 0
		};
		device->AttachFramebufferTexture2D(&attachTexture);

		RenderCommandData::ViewportData viewport{ 0, 0, LutSize, LutSize };
		device->Viewport(&viewport);

		ConstStringView path("engine/shaders/preprocess/calc_brdf_lut.glsl");
		ShaderId calcBrdfShaderId = parameters.shaderManager->FindShaderByPath(path);
		const ShaderData& calcBrdfShader = parameters.shaderManager->GetShaderData(calcBrdfShaderId);
		device->UseShaderProgram(calcBrdfShader.driverId);

		const MeshDrawData* meshDraw = parameters.meshManager->GetDrawData(meshId);
		device->BindVertexArray(meshDraw->vertexArrayObject);
		device->DrawIndexed(meshDraw->primitiveMode, meshDraw->count, meshDraw->indexType);

		device->DestroyFramebuffers(1, &framebuffer);

		kokko::ConstStringView label("Renderer BRDF LUT");
		device->SetObjectLabel(RenderObjectType::Texture, brdfLutTextureId, label);
	}
}

void GraphicsFeatureDeferredLighting::Deinitialize(const InitializeParameters& parameters)
{
	parameters.renderDevice->DestroyBuffers(1, &uniformBufferId);
	uniformBufferId = 0;

	parameters.renderDevice->DestroyTextures(1, &brdfLutTextureId);
	brdfLutTextureId = 0;

	parameters.meshManager->RemoveMesh(meshId);
	meshId = MeshId::Null;
}

void GraphicsFeatureDeferredLighting::Submit(const SubmitParameters& parameters)
{
	parameters.commandList.AddToFullscreenViewportWithOrder(RenderPass::OpaqueLighting, renderOrder, 0);
}

void GraphicsFeatureDeferredLighting::Render(const RenderParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	RenderDevice* device = parameters.renderDevice;
	LightManager* lightManager = parameters.lightManager;

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

			lightingUniforms.lightColors[i] = lightManager->GetColor(dirLightId);
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

		const size_t dirLightOffset = 1;
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
				shaderLightIdx = dirLightOffset + pointLightCount + spotLightsAdded;
				spotLightsAdded += 1;

				Mat3x3f orientation = lightManager->GetOrientation(lightId);
				Vec3f wLightDir = orientation * Vec3f(0.0f, 0.0f, -1.0f);
				Vec4f vLightDir = fsvp.view.inverse * Vec4f(wLightDir, 0.0f);
				vLightDir.w = lightManager->GetSpotAngle(lightId);
				lightingUniforms.lightDirections[shaderLightIdx] = vLightDir;
			}
			else
			{
				shaderLightIdx = dirLightOffset + pointLightsAdded;
				pointLightsAdded += 1;
			}

			Vec3f wLightPos = lightManager->GetPosition(lightId);
			Vec3f vLightPos = (fsvp.view.inverse * Vec4f(wLightPos, 1.0f)).xyz();

			float radius = lightManager->GetRadius(lightId);
			float inverseSquareRadius = 1.0f / (radius * radius);
			lightingUniforms.lightPositions[shaderLightIdx] = Vec4f(vLightPos, inverseSquareRadius);

			Vec3f lightCol = lightManager->GetColor(lightId);
			lightingUniforms.lightColors[shaderLightIdx] = lightCol;
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

		device->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferId);
		device->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(LightingUniformBlock), &lightingUniforms);
	}

	device->DepthTestDisable();

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

	deferredPass.samplerIds[0] = 0;
	deferredPass.samplerIds[1] = 0;
	deferredPass.samplerIds[2] = 0;
	deferredPass.samplerIds[3] = 0;
	deferredPass.samplerIds[4] = 0;
	deferredPass.samplerIds[5] = 0;
	deferredPass.samplerIds[6] = 0;
	deferredPass.samplerIds[7] = 0;
	deferredPass.samplerIds[8] = 0;

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
