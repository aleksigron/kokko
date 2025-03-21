#include "Graphics/EnvironmentSystem.hpp"

#include <cassert>
#include <cstdint>

#include "stb_image/stb_image.h"

#include "Core/Array.hpp"
#include "Core/Core.hpp"

#include "Math/Mat4x4.hpp"
#include "Math/Math.hpp"
#include "Math/Projection.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/CommandEncoder.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/Uniform.hpp"

#include "Resources/AssetLoader.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ModelManager.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"


namespace kokko
{

namespace
{

RenderTextureTarget GetCubeTextureTarget(unsigned int index)
{
	// TODO: Refactor into a reusable function
	static RenderTextureTarget cubeTextureTargets[] = {
		RenderTextureTarget::TextureCubeMap_PositiveX,
		RenderTextureTarget::TextureCubeMap_NegativeX,
		RenderTextureTarget::TextureCubeMap_PositiveY,
		RenderTextureTarget::TextureCubeMap_NegativeY,
		RenderTextureTarget::TextureCubeMap_PositiveZ,
		RenderTextureTarget::TextureCubeMap_NegativeZ
	};

	return cubeTextureTargets[index];
}

void ReleaseEnvTextures(kokko::TextureManager* textureManager, kokko::EnvironmentTextures& textures)
{
	auto releaseTexture = [textureManager](kokko::TextureId& texture)
	{
		if (texture != kokko::TextureId::Null)
		{
			textureManager->RemoveTexture(texture);
			texture = kokko::TextureId::Null;
		}
	};

	releaseTexture(textures.environmentTexture);
	releaseTexture(textures.diffuseIrradianceTexture);
	releaseTexture(textures.specularIrradianceTexture);
}

void BindTexture(kokko::render::CommandEncoder* encoder, const kokko::ShaderData& shader,
	uint32_t uniformHash, kokko::render::TextureId textureId)
{
	const kokko::TextureUniform* uniform = shader.uniforms.FindTextureUniformByNameHash(uniformHash);
	if (uniform != nullptr)
	{
		encoder->BindTextureToShader(uniform->uniformLocation, 0, textureId);
	}
}

void BindBufferRange(kokko::render::CommandEncoder* encoder, unsigned int binding, kokko::render::BufferId buffer, intptr_t offset, size_t size)
{
	encoder->BindBufferRange(RenderBufferTarget::UniformBuffer, binding, buffer, offset, size);
}

} // Anonymous namespace

const EnvironmentId EnvironmentId::Null = EnvironmentId{ 0 };

struct CalcSpecularUniforms
{
	alignas(16) float roughness;
};

EnvironmentSystem::EnvironmentSystem(
	Allocator* allocator,
	AssetLoader* assetLoader,
	render::Device* renderDevice,
	ShaderManager* shaderManager,
	ModelManager* modelManager,
	TextureManager* textureManager) :
	allocator(allocator),
	assetLoader(assetLoader),
	renderDevice(renderDevice),
	shaderManager(shaderManager),
	modelManager(modelManager),
	textureManager(textureManager),
	components(allocator),
	texturesToRemove(allocator),
	entityMap(allocator),
	viewportBlockStride(0),
	specularBlockStride(0),
	framebufferIds(allocator),
	viewportUniformBufferId(0),
	specularUniformBufferId(0),
	samplerId(0),
	cubeMeshId(ModelId::Null),
	resourcesUploaded(false)
{
	components.Reserve(4);
	components.PushBack(); // Reserve index 0 as EnvironmentId::Null
}

EnvironmentSystem::~EnvironmentSystem()
{
	Deinitialize();
}

void EnvironmentSystem::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

#ifdef KOKKO_USE_METAL
    return;
#endif
}

void EnvironmentSystem::Deinitialize()
{
	if (resourcesUploaded == true)
	{
		resourcesUploaded = false;

		renderDevice->DestroySamplers(1, &samplerId);
		samplerId = render::SamplerId();

		renderDevice->DestroyBuffers(1, &specularUniformBufferId);
		specularUniformBufferId = render::BufferId();

		renderDevice->DestroyBuffers(1, &viewportUniformBufferId);
		viewportUniformBufferId = render::BufferId();

		renderDevice->DestroyFramebuffers(framebufferIds.GetCount(), framebufferIds.GetData());
		framebufferIds.Clear();

		modelManager->RemoveModel(cubeMeshId);
		cubeMeshId = ModelId::Null;

		ReleaseEnvTextures(textureManager, emptyEnvironmentMap);
	}

	for (size_t i = 1, count = components.GetCount(); i < count; ++i)
	{
		ReleaseEnvTextures(textureManager, components[i].textures);
	}

	components.Resize(1);
}

void EnvironmentSystem::Upload(render::CommandEncoder* encoder)
{
#ifdef KOKKO_USE_METAL
    return;
#endif

	if (texturesToRemove.GetCount() != 0)
	{
		renderDevice->DestroyTextures(texturesToRemove.GetCount(), texturesToRemove.GetData());
		texturesToRemove.Clear();
	}

	if (resourcesUploaded == false)
	{
		resourcesUploaded = true;

		auto scope = renderDevice->CreateDebugScope(0, ConstStringView("EnvironSys_InitResources"));

		// Create cube mesh
		cubeMeshId = MeshPresets::CreateCube(modelManager);

		// Create framebuffer

		constexpr uint32_t totalFramebufferCount = CubemapSideCount * 2 + CubemapSideCount * SpecularMipmapLevelCount;
		framebufferIds.Resize(totalFramebufferCount);
		renderDevice->CreateFramebuffers(totalFramebufferCount, framebufferIds.GetData());

		// Create viewport uniform buffer

		ProjectionParameters projection;
		projection.perspectiveNear = 0.1f;
		projection.perspectiveFar = 1.0f;
		projection.aspect = 1.0f;
		projection.SetPerspective(Math::DegreesToRadians(90.0f));
		Mat4x4f projectionMatrix = projection.GetProjectionMatrix(false);

		Vec3f zero3(0.0f, 0.0f, 0.0f);
		Mat4x4f viewTransforms[CubemapSideCount] = {
			Mat4x4f::LookAt(zero3, Vec3f(1.0f, 0.0f, 0.0f), Vec3f(0.0f, -1.0f, 0.0f)).GetInverseNonScaled(),
			Mat4x4f::LookAt(zero3, Vec3f(-1.0f, 0.0f, 0.0f), Vec3f(0.0f, -1.0f, 0.0f)).GetInverseNonScaled(),
			Mat4x4f::LookAt(zero3, Vec3f(0.0f, 1.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f)).GetInverseNonScaled(),
			Mat4x4f::LookAt(zero3, Vec3f(0.0f, -1.0f, 0.0f), Vec3f(0.0f, 0.0f, -1.0f)).GetInverseNonScaled(),
			Mat4x4f::LookAt(zero3, Vec3f(0.0f, 0.0f, 1.0f), Vec3f(0.0f, -1.0f, 0.0f)).GetInverseNonScaled(),
			Mat4x4f::LookAt(zero3, Vec3f(0.0f, 0.0f, -1.0f), Vec3f(0.0f, -1.0f, 0.0f)).GetInverseNonScaled()
		};

		int alignment;
		renderDevice->GetIntegerValue(RenderDeviceParameter::UniformBufferOffsetAlignment, &alignment);

		Array<unsigned char> uniformBuffer(allocator);

		// Viewport uniforms

		viewportBlockStride = Math::RoundUpToMultiple(sizeof(TransformUniformBlock), size_t(alignment));
		unsigned int viewportBufferSize = static_cast<unsigned int>(viewportBlockStride * CubemapSideCount);

		uniformBuffer.Resize(viewportBufferSize);

		for (size_t i = 0; i < CubemapSideCount; ++i)
		{
			ViewportUniformBlock* uniforms = reinterpret_cast<ViewportUniformBlock*>(&uniformBuffer[viewportBlockStride * i]);

			uniforms->V = viewTransforms[i];
			uniforms->P = projectionMatrix;
			uniforms->VP = projectionMatrix * viewTransforms[i];
		}

		renderDevice->CreateBuffers(1, &viewportUniformBufferId);
		renderDevice->SetBufferStorage(
			viewportUniformBufferId, viewportBufferSize, uniformBuffer.GetData(), BufferStorageFlags{});

		// Calculate specular uniforms

		specularBlockStride = Math::RoundUpToMultiple(sizeof(CalcSpecularUniforms), size_t(alignment));
		unsigned int specularBufferSize = static_cast<unsigned int>(specularBlockStride * SpecularMipmapLevelCount);

		uniformBuffer.Resize(specularBufferSize);

		for (size_t mip = 0; mip < SpecularMipmapLevelCount; ++mip)
		{
			CalcSpecularUniforms* uniforms = reinterpret_cast<CalcSpecularUniforms*>(&uniformBuffer[specularBlockStride * mip]);

			uniforms->roughness = mip / static_cast<float>(SpecularMipmapLevelCount - 1);
		}

		renderDevice->CreateBuffers(1, &specularUniformBufferId);
		renderDevice->SetBufferStorage(
			specularUniformBufferId, specularBufferSize, uniformBuffer.GetData(), BufferStorageFlags{});

		// Create sampler object

		RenderSamplerParameters samplerParams{
			RenderTextureFilterMode::Linear,
			RenderTextureFilterMode::Linear,
			RenderTextureWrapMode::ClampToEdge,
			RenderTextureWrapMode::ClampToEdge,
			RenderTextureWrapMode::ClampToEdge,
			RenderTextureCompareMode::None,
			RenderDepthCompareFunc::Never
		};
		renderDevice->CreateSamplers(1, &samplerParams, &samplerId);

		{
			// Upload empty environment map

			static const int EmptyTextureSize = 1 << SpecularMipmapLevelCount;
			static const unsigned char EmptyTextureBytes[EmptyTextureSize * EmptyTextureSize * 3] = { 0 };

			RenderTextureSizedFormat sizedFormat = RenderTextureSizedFormat::RGB8;

			// Create environment texture

			Vec2i envMapSize(EmptyTextureSize, EmptyTextureSize);
			TextureId envMapTextureId = textureManager->CreateTexture();
			textureManager->AllocateTextureStorage(envMapTextureId, RenderTextureTarget::TextureCubeMap, sizedFormat, 1, envMapSize);
			const TextureData& envMapTexture = textureManager->GetTextureData(envMapTextureId);

			for (unsigned int i = 0; i < CubemapSideCount; ++i)
			{
				renderDevice->SetTextureSubImage3D(envMapTexture.textureObjectId, 0, 0, 0, i,
					EmptyTextureSize, EmptyTextureSize, 1, RenderTextureBaseFormat::RGB,
					RenderTextureDataType::UnsignedByte, EmptyTextureBytes);
			}

			// Create diffuse irradiance texture

			Vec2i diffuseMapSize(EmptyTextureSize, EmptyTextureSize);
			TextureId diffuseMapTextureId = textureManager->CreateTexture();
			textureManager->AllocateTextureStorage(diffuseMapTextureId, RenderTextureTarget::TextureCubeMap, sizedFormat, 1, diffuseMapSize);
			const TextureData& diffuseMapTexture = textureManager->GetTextureData(diffuseMapTextureId);

			for (unsigned int i = 0; i < CubemapSideCount; ++i)
			{
				renderDevice->SetTextureSubImage3D(diffuseMapTexture.textureObjectId,
					0, 0, 0, i, EmptyTextureSize, EmptyTextureSize, 1, RenderTextureBaseFormat::RGB,
					RenderTextureDataType::UnsignedByte, EmptyTextureBytes);
			}

			// Create specular irradiance texture

			Vec2i specMapSize(EmptyTextureSize, EmptyTextureSize);
			TextureId specMapTextureId = textureManager->CreateTexture();
			textureManager->AllocateTextureStorage(specMapTextureId, RenderTextureTarget::TextureCubeMap,
				sizedFormat, SpecularMipmapLevelCount, specMapSize);
			const TextureData& specMapTexture = textureManager->GetTextureData(specMapTextureId);

			for (int mip = 0; mip < SpecularMipmapLevelCount; ++mip)
			{
				int mipSize = EmptyTextureSize >> mip;

				for (unsigned int i = 0; i < CubemapSideCount; ++i)
				{
					renderDevice->SetTextureSubImage3D(specMapTexture.textureObjectId,
						mip, 0, 0, i, mipSize, mipSize, 1, RenderTextureBaseFormat::RGB,
						RenderTextureDataType::UnsignedByte, EmptyTextureBytes);
				}
			}

			emptyEnvironmentMap.environmentTexture = envMapTextureId;
			emptyEnvironmentMap.diffuseIrradianceTexture = diffuseMapTextureId;
			emptyEnvironmentMap.specularIrradianceTexture = specMapTextureId;
		}
	}

	// Upload any environment textures that need it
	// TODO: Do rendering in a separate function

	for (size_t i = 0, end = components.GetCount(); i < end; ++i)
	{
		auto& env = components[i];
		if (env.needsUpload && env.sourceTextureUid.HasValue())
		{
			static const int EnvironmentTextureSize = 1024;
			static const int SpecularTextureSize = 256;
			static const int DiffuseTextureSize = 32;

			RenderTextureSizedFormat sizedFormat = RenderTextureSizedFormat::RGB16F;

			// Load equirectangular image and create texture

			stbi_set_flip_vertically_on_load(true);
			int equirectWidth, equirectHeight, nrComponents;
			float* equirectData;

			{
				Array<uint8_t> buffer(allocator);
				AssetLoader::LoadResult loadResult = assetLoader->LoadAsset(env.sourceTextureUid.GetValue(), buffer);
				if (loadResult.success == false)
				{
					KK_LOG_ERROR("Couldn't read source texture for environment map");
					continue;
				}

				{
					KOKKO_PROFILE_SCOPE("stbi_loadf_from_memory()");

					uint8_t* bytesPtr = buffer.GetData() + loadResult.assetStart;
					int length = static_cast<int>(loadResult.assetSize);
					equirectData = stbi_loadf_from_memory(bytesPtr, length, &equirectWidth, &equirectHeight, &nrComponents, 0);
				}
			}

			if (equirectData == nullptr)
			{
				KK_LOG_ERROR("Couldn't load source texture for environment map");
				continue;
			}

			auto devScope = renderDevice->CreateDebugScope(0, ConstStringView("EnvironSys_CreateMapResources"));
			auto encScope = encoder->CreateDebugScope(0, ConstStringView("EnvironSys_RenderMapResources"));

			kokko::render::TextureId equirectTextureId;
			renderDevice->CreateTextures(RenderTextureTarget::Texture2d, 1, &equirectTextureId);
			renderDevice->SetTextureStorage2D(equirectTextureId, 1, sizedFormat, equirectWidth, equirectHeight);
			renderDevice->SetTextureSubImage2D(equirectTextureId, 0, 0, 0, equirectWidth, equirectHeight,
				RenderTextureBaseFormat::RGB, RenderTextureDataType::Float, equirectData);

			stbi_image_free(equirectData);

			// Create result cubemap texture

			uint32_t usedFramebufferCount = 0;
			Vec2i envMapSize(EnvironmentTextureSize, EnvironmentTextureSize);
			TextureId envMapTextureId = textureManager->CreateTexture();
			textureManager->AllocateTextureStorage(envMapTextureId, RenderTextureTarget::TextureCubeMap, sizedFormat, 1, envMapSize);
			const TextureData& envMapTexture = textureManager->GetTextureData(envMapTextureId);

			encoder->BlendingDisable();
			encoder->ScissorTestDisable();
			encoder->BindSampler(0, samplerId);

			auto& mesh = modelManager->GetModelMeshes(cubeMeshId)[0];
			auto& part = modelManager->GetModelMeshParts(cubeMeshId)[0];

			// Load shader

			ShaderId equirectShaderId = shaderManager->FindShaderByPath(ConstStringView("engine/shaders/preprocess/equirect_to_cube.glsl"));
			const ShaderData& equirectShader = shaderManager->GetShaderData(equirectShaderId);

			encoder->BindVertexArray(part.vertexArrayId);
			encoder->UseShaderProgram(equirectShader.driverId);

			BindTexture(encoder, equirectShader, "equirectangular_map"_hash, equirectTextureId);

			// Render
			{
				KOKKO_PROFILE_SCOPE("Mesh equirect to cubemap");

				encoder->SetViewport(0, 0, EnvironmentTextureSize, EnvironmentTextureSize);

				for (uint32_t i = 0; i < CubemapSideCount; ++i)
				{
					render::FramebufferId framebuffer = framebufferIds[usedFramebufferCount];
					usedFramebufferCount += 1;

					renderDevice->AttachFramebufferTextureLayer(
						framebuffer, RenderFramebufferAttachment::Color0, envMapTexture.textureObjectId, 0, i);

					encoder->BindFramebuffer(framebuffer);

					BindBufferRange(encoder, UniformBlockBinding::Viewport,
						viewportUniformBufferId, viewportBlockStride * i, sizeof(ViewportUniformBlock));

					encoder->DrawIndexed(mesh.primitiveMode, mesh.indexType, part.count, part.indexOffset, 0);
				}
			}

			texturesToRemove.PushBack(equirectTextureId);
			equirectTextureId = render::TextureId();

			// Calculate diffuse irradiance

			// Create texture

			Vec2i diffuseMapSize(DiffuseTextureSize, DiffuseTextureSize);
			TextureId diffuseMapTextureId = textureManager->CreateTexture();
			textureManager->AllocateTextureStorage(diffuseMapTextureId, RenderTextureTarget::TextureCubeMap, sizedFormat, 1, diffuseMapSize);
			const TextureData& diffuseMapTexture = textureManager->GetTextureData(diffuseMapTextureId);

			// Load shader

			ShaderId calcDiffuseShaderId = shaderManager->FindShaderByPath(ConstStringView("engine/shaders/preprocess/calc_diffuse_irradiance.glsl"));
			const ShaderData& calcDiffuseShader = shaderManager->GetShaderData(calcDiffuseShaderId);

			encoder->UseShaderProgram(calcDiffuseShader.driverId);

			BindTexture(encoder, calcDiffuseShader, "environment_map"_hash, envMapTexture.textureObjectId);

			{
				KOKKO_PROFILE_SCOPE("Convolute to diffuse irradiance map");

				encoder->SetViewport(0, 0, DiffuseTextureSize, DiffuseTextureSize);

				for (uint32_t i = 0; i < CubemapSideCount; ++i)
				{
					render::FramebufferId framebuffer = framebufferIds[usedFramebufferCount];
					usedFramebufferCount += 1;

					renderDevice->AttachFramebufferTextureLayer(
						framebuffer, RenderFramebufferAttachment::Color0, diffuseMapTexture.textureObjectId, 0, i);

					encoder->BindFramebuffer(framebuffer);

					BindBufferRange(encoder, UniformBlockBinding::Viewport,
						viewportUniformBufferId, viewportBlockStride * i, sizeof(ViewportUniformBlock));

					encoder->DrawIndexed(mesh.primitiveMode, mesh.indexType, part.count, part.indexOffset, 0);
				}
			}

			// Calculate specular irradiance

			// Create texture

			Vec2i specMapSize(SpecularTextureSize, SpecularTextureSize);
			TextureId specMapTextureId = textureManager->CreateTexture();
			textureManager->AllocateTextureStorage(specMapTextureId, RenderTextureTarget::TextureCubeMap,
				sizedFormat, SpecularMipmapLevelCount, specMapSize);

			const TextureData& specMapTexture = textureManager->GetTextureData(specMapTextureId);

			ShaderId calcSpecularShaderId = shaderManager->FindShaderByPath(ConstStringView("engine/shaders/preprocess/calc_specular_irradiance.glsl"));
			const ShaderData& calcSpecularShader = shaderManager->GetShaderData(calcSpecularShaderId);

			encoder->UseShaderProgram(calcSpecularShader.driverId);

			BindTexture(encoder, calcSpecularShader, "environment_map"_hash, envMapTexture.textureObjectId);

			{
				KOKKO_PROFILE_SCOPE("Prefilter specular map");

				for (uint32_t mip = 0; mip < SpecularMipmapLevelCount; ++mip)
				{
					BindBufferRange(encoder, UniformBlockBinding::Object,
						specularUniformBufferId, specularBlockStride * mip, sizeof(CalcSpecularUniforms));

					// Resize framebuffer according to mip-level size.
					unsigned int mipSize = static_cast<unsigned int>(SpecularTextureSize) >> mip;

					encoder->SetViewport(0, 0, static_cast<int>(mipSize), static_cast<int>(mipSize));

					for (uint32_t i = 0; i < CubemapSideCount; ++i)
					{
						render::FramebufferId framebuffer = framebufferIds[usedFramebufferCount];
						usedFramebufferCount += 1;

						renderDevice->AttachFramebufferTextureLayer(
							framebuffer, RenderFramebufferAttachment::Color0, specMapTexture.textureObjectId, mip, i);

						encoder->BindFramebuffer(framebuffer);

						BindBufferRange(encoder, UniformBlockBinding::Viewport,
							viewportUniformBufferId, viewportBlockStride * i, sizeof(ViewportUniformBlock));

						encoder->DrawIndexed(mesh.primitiveMode, mesh.indexType, part.count, part.indexOffset, 0);
					}
				}
			}

			assert(usedFramebufferCount == framebufferIds.GetCount());

			encoder->BindSampler(0, render::SamplerId());

			env.textures.environmentTexture = envMapTextureId;
			env.textures.diffuseIrradianceTexture = diffuseMapTextureId;
			env.textures.specularIrradianceTexture = specMapTextureId;
			env.needsUpload = false;

			// Only render one environment map per frame
			break;
		}
	}
}

EnvironmentId EnvironmentSystem::Lookup(Entity entity)
{
	auto* pair = entityMap.Lookup(entity.id);
	return pair != nullptr ? pair->second : EnvironmentId{};
}

EnvironmentId EnvironmentSystem::AddComponent(Entity entity)
{
	EnvironmentId id{ static_cast<unsigned int>(components.GetCount()) };

	auto mapPair = entityMap.Insert(entity.id);
	mapPair->second = id;

	components.PushBack();
	components[id.i].entity = entity;
	components[id.i].intensity = 1.0f;
	components[id.i].needsUpload = false;

	return id;
}

void EnvironmentSystem::RemoveComponent(EnvironmentId id)
{
	assert(id != EnvironmentId::Null);
	assert(id.i < components.GetCount());

	// Release textures
	textureManager->RemoveTexture(components[id.i].textures.environmentTexture);
	textureManager->RemoveTexture(components[id.i].textures.diffuseIrradianceTexture);
	textureManager->RemoveTexture(components[id.i].textures.specularIrradianceTexture);

	components[id.i].textures = EnvironmentTextures();

	// Remove from entity map
	Entity entity = components[id.i].entity;
	auto* pair = entityMap.Lookup(entity.id);
	if (pair != nullptr)
		entityMap.Remove(pair);

	if (components.GetCount() > 2 && id.i + 1 < components.GetCount()) // We need to swap another object
	{
		size_t swapIdx = components.GetCount() - 1;

		// Update the swapped objects id in the entity map
		auto* swapKv = entityMap.Lookup(components[swapIdx].entity.id);
		if (swapKv != nullptr)
			swapKv->second = id;

		components[id.i].entity = components[swapIdx].entity;
		components[id.i].sourceTextureUid = components[swapIdx].sourceTextureUid;
		components[id.i].textures = components[swapIdx].textures;
	}

	components.PopBack();
}

void EnvironmentSystem::RemoveAll()
{
	for (size_t i = 1, count = components.GetCount(); i < count; ++i)
	{
		ReleaseEnvTextures(textureManager, components[i].textures);
	}

	components.Resize(1);
	entityMap.Clear();
}

void EnvironmentSystem::SetEnvironmentTexture(EnvironmentId id, const kokko::Uid& textureUid)
{
	KOKKO_PROFILE_FUNCTION();

	assert(id != EnvironmentId::Null);

	auto& component = components[id.i];

	if (component.sourceTextureUid.HasValue() &&
		component.sourceTextureUid.GetValue() == textureUid)
		return;

	// Release previous environment textures
	ReleaseEnvTextures(textureManager, component.textures);

	component.sourceTextureUid = textureUid;
	component.needsUpload = true;
}

void EnvironmentSystem::SetIntensity(EnvironmentId id, float intensity)
{
	assert(id != EnvironmentId::Null);
	auto& component = components[id.i];
	component.intensity = intensity;
}

EnvironmentId EnvironmentSystem::FindActiveEnvironment()
{
	for (size_t i = 1, count = components.GetCount(); i < count; ++i)
	{
		if (components[i].sourceTextureUid.HasValue())
			return EnvironmentId{ static_cast<unsigned int>(i) };
	}

	return EnvironmentId::Null;
}

EnvironmentTextures EnvironmentSystem::GetEnvironmentMap(EnvironmentId id) const
{
	assert(id != EnvironmentId::Null);
	return components[id.i].textures;
}

Optional<Uid> EnvironmentSystem::GetSourceTextureUid(EnvironmentId id) const
{
	assert(id != EnvironmentId::Null);
	return components[id.i].sourceTextureUid;
}

float EnvironmentSystem::GetIntensity(EnvironmentId id) const
{
	assert(id != EnvironmentId::Null);
	return components[id.i].intensity;
}

EnvironmentTextures EnvironmentSystem::GetEmptyEnvironmentMap() const
{
	return emptyEnvironmentMap;
}

void EnvironmentSystem::LoadEmptyEnvironmentMap()
{
	KOKKO_PROFILE_FUNCTION();
}

} // namespace kokko
