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

#include "Rendering/RenderDevice.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/Uniform.hpp"

#include "Resources/AssetLoader.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"

namespace
{

void ReleaseEnvTextures(TextureManager* textureManager, kokko::EnvironmentTextures& textures)
{
	auto releaseTexture = [textureManager](TextureId& texture)
	{
		if (texture != TextureId::Null)
		{
			textureManager->RemoveTexture(texture);
			texture = TextureId::Null;
		}
	};

	releaseTexture(textures.environmentTexture);
	releaseTexture(textures.diffuseIrradianceTexture);
	releaseTexture(textures.specularIrradianceTexture);
}
} // Anonymous namespace

namespace kokko
{

const EnvironmentId EnvironmentId::Null = EnvironmentId{ 0 };

struct CalcSpecularUniforms
{
	alignas(16) float roughness;
};

static RenderTextureTarget GetCubeTextureTarget(unsigned int index)
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

EnvironmentSystem::EnvironmentSystem(
	Allocator* allocator,
	kokko::AssetLoader* assetLoader,
	RenderDevice* renderDevice,
	ShaderManager* shaderManager,
	MeshManager* meshManager,
	TextureManager* textureManager) :
	allocator(allocator),
	assetLoader(assetLoader),
	renderDevice(renderDevice),
	shaderManager(shaderManager),
	meshManager(meshManager),
	textureManager(textureManager),
	environmentMaps(allocator),
	entityMap(allocator),
	viewportBlockStride(0),
	specularBlockStride(0),
	framebufferId(0),
	viewportUniformBufferId(0),
	specularUniformBufferId(0),
	samplerId(0),
	cubeMeshId(MeshId::Null)
{
	environmentMaps.Reserve(4);
	environmentMaps.PushBack(); // Reserve index 0 as EnvironmentId::Null
}

EnvironmentSystem::~EnvironmentSystem()
{
	Deinitialize();
}

void EnvironmentSystem::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	// Create cube mesh

	cubeMeshId = meshManager->CreateMesh();
	MeshPresets::UploadCube(meshManager, cubeMeshId);

	// Create framebuffer

	renderDevice->CreateFramebuffers(1, &framebufferId);

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

	viewportBlockStride = (sizeof(TransformUniformBlock) + alignment - 1) / alignment * alignment;
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
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, viewportUniformBufferId);

	RenderCommandData::SetBufferStorage viewportBufferStorage{
		RenderBufferTarget::UniformBuffer, viewportBufferSize, uniformBuffer.GetData()
	};
	renderDevice->SetBufferStorage(&viewportBufferStorage);

	// Calculate specular uniforms

	specularBlockStride = (sizeof(CalcSpecularUniforms) + alignment - 1) / alignment * alignment;
	unsigned int specularBufferSize = static_cast<unsigned int>(specularBlockStride * SpecularMipmapLevelCount);

	uniformBuffer.Resize(specularBufferSize);

	for (size_t mip = 0; mip < SpecularMipmapLevelCount; ++mip)
	{
		CalcSpecularUniforms* uniforms = reinterpret_cast<CalcSpecularUniforms*>(&uniformBuffer[specularBlockStride * mip]);

		uniforms->roughness = mip / static_cast<float>(SpecularMipmapLevelCount - 1);
	}

	renderDevice->CreateBuffers(1, &specularUniformBufferId);
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, specularUniformBufferId);

	RenderCommandData::SetBufferStorage specularBufferStorage{
		RenderBufferTarget::UniformBuffer, specularBufferSize, uniformBuffer.GetData()
	};
	renderDevice->SetBufferStorage(&specularBufferStorage);

	// Create sampler object

	renderDevice->CreateSamplers(1, &samplerId);
	RenderCommandData::SetSamplerParameters samplerParams{
		samplerId, RenderTextureFilterMode::Linear, RenderTextureFilterMode::Linear,
		RenderTextureWrapMode::ClampToEdge, RenderTextureWrapMode::ClampToEdge, RenderTextureWrapMode::ClampToEdge,
		RenderTextureCompareMode::None
	};
	renderDevice->SetSamplerParameters(&samplerParams);

	LoadEmptyEnvironmentMap();
}

void EnvironmentSystem::Deinitialize()
{
	if (samplerId != 0)
	{
		renderDevice->DestroySamplers(1, &samplerId);
		samplerId = 0;
	}

	if (specularUniformBufferId != 0)
	{
		renderDevice->DestroyBuffers(1, &specularUniformBufferId);
		specularUniformBufferId = 0;
	}

	if (viewportUniformBufferId != 0)
	{
		renderDevice->DestroyBuffers(1, &viewportUniformBufferId);
		viewportUniformBufferId = 0;
	}

	if (framebufferId != 0)
	{
		renderDevice->DestroyFramebuffers(1, &framebufferId);
		framebufferId = 0;
	}

	if (cubeMeshId != MeshId::Null)
	{
		meshManager->RemoveMesh(cubeMeshId);
		cubeMeshId = MeshId::Null;
	}

	for (size_t i = 1, count = environmentMaps.GetCount(); i < count; ++i)
	{
		ReleaseEnvTextures(textureManager, environmentMaps[i].textures);
	}
}

EnvironmentId EnvironmentSystem::Lookup(Entity entity)
{
	auto* pair = entityMap.Lookup(entity.id);
	return pair != nullptr ? pair->second : EnvironmentId{};
}

EnvironmentId EnvironmentSystem::AddComponent(Entity entity)
{
	EnvironmentId id{ static_cast<unsigned int>(environmentMaps.GetCount()) };

	auto mapPair = entityMap.Insert(entity.id);
	mapPair->second = id;

	environmentMaps.PushBack();
	environmentMaps[id.i].entity = entity;

	return id;
}

void EnvironmentSystem::RemoveComponent(EnvironmentId id)
{
	assert(id != EnvironmentId::Null);
	assert(id.i < environmentMaps.GetCount());

	// Release textures
	textureManager->RemoveTexture(environmentMaps[id.i].textures.environmentTexture);
	textureManager->RemoveTexture(environmentMaps[id.i].textures.diffuseIrradianceTexture);
	textureManager->RemoveTexture(environmentMaps[id.i].textures.specularIrradianceTexture);

	environmentMaps[id.i].textures = EnvironmentTextures();

	// Remove from entity map
	Entity entity = environmentMaps[id.i].entity;
	auto* pair = entityMap.Lookup(entity.id);
	if (pair != nullptr)
		entityMap.Remove(pair);

	if (environmentMaps.GetCount() > 2 && id.i + 1 < environmentMaps.GetCount()) // We need to swap another object
	{
		size_t swapIdx = environmentMaps.GetCount() - 1;

		// Update the swapped objects id in the entity map
		auto* swapKv = entityMap.Lookup(environmentMaps[swapIdx].entity.id);
		if (swapKv != nullptr)
			swapKv->second = id;

		environmentMaps[id.i].entity = environmentMaps[swapIdx].entity;
		environmentMaps[id.i].sourceTextureUid = environmentMaps[swapIdx].sourceTextureUid;
		environmentMaps[id.i].textures = environmentMaps[swapIdx].textures;
	}

	environmentMaps.PopBack();
}

void EnvironmentSystem::RemoveAll()
{
	for (size_t i = 1, count = environmentMaps.GetCount(); i < count; ++i)
	{
		ReleaseEnvTextures(textureManager, environmentMaps[i].textures);
	}

	environmentMaps.Resize(1);
	entityMap.Clear();
}

static void BindTexture(RenderDevice* renderDevice, const ShaderData& shader,
	uint32_t uniformHash, RenderTextureTarget target, unsigned int textureId)
{
	const kokko::TextureUniform* uniform = shader.uniforms.FindTextureUniformByNameHash(uniformHash);
	if (uniform != nullptr)
	{
		renderDevice->SetUniformInt(uniform->uniformLocation, 0);
		renderDevice->SetActiveTextureUnit(0);
		renderDevice->BindTexture(target, textureId);
	}
}

static void SetViewport(RenderDevice* renderDevice, int size)
{
	RenderCommandData::ViewportData viewport{ 0, 0, size, size };
	renderDevice->Viewport(&viewport);
}

static void BindBufferRange(RenderDevice* renderDevice, unsigned int binding, unsigned int buffer, intptr_t offset, size_t size)
{
	RenderCommandData::BindBufferRange bindBufferRange{ RenderBufferTarget::UniformBuffer, binding, buffer, offset, size };
	renderDevice->BindBufferRange(&bindBufferRange);
}

static void AttachFramebufferTexture(RenderDevice* renderDevice, unsigned int faceIndex, unsigned int textureId, int mip)
{
	RenderCommandData::AttachFramebufferTexture2D attachFramebufferTexture{
		RenderFramebufferTarget::Framebuffer, RenderFramebufferAttachment::Color0,
		GetCubeTextureTarget(faceIndex), textureId, mip
	};
	renderDevice->AttachFramebufferTexture2D(&attachFramebufferTexture);
}

void EnvironmentSystem::SetEnvironmentTexture(EnvironmentId id, const kokko::Uid& textureUid)
{
	KOKKO_PROFILE_FUNCTION();

	assert(id != EnvironmentId::Null);

	if (environmentMaps[id.i].sourceTextureUid.HasValue() &&
		environmentMaps[id.i].sourceTextureUid.GetValue() == textureUid)
		return;

	// Release previous environment textures
	ReleaseEnvTextures(textureManager, environmentMaps[id.i].textures);

	static const int EnvironmentTextureSize = 1024;
	static const int SpecularTextureSize = 256;
	static const int DiffuseTextureSize = 32;

	RenderTextureSizedFormat sizedFormat = RenderTextureSizedFormat::RGB16F;

	// Load equirectangular image and create texture

	stbi_set_flip_vertically_on_load(true);
	int equirectWidth, equirectHeight, nrComponents;
	float* equirectData;

	{
		Array<uint8_t> fileBytes(allocator);

		if (assetLoader->LoadAsset(textureUid, fileBytes) == false)
		{
			KK_LOG_ERROR("Couldn't read source texture for environment map");
			return;
		}

		{
			KOKKO_PROFILE_SCOPE("stbi_loadf_from_memory()");

			uint8_t* fileBytesPtr = fileBytes.GetData();
			int length = static_cast<int>(fileBytes.GetCount());
			equirectData = stbi_loadf_from_memory(fileBytesPtr, length, &equirectWidth, &equirectHeight, &nrComponents, 0);
		}
	}

	if (equirectData == nullptr)
	{
		KK_LOG_ERROR("Couldn't load source texture for environment map");
		return;
	}

	unsigned int equirectTextureId = 0;

	{
		KOKKO_PROFILE_SCOPE("Upload equirectangular environment map");

		renderDevice->CreateTextures(1, &equirectTextureId);
		renderDevice->BindTexture(RenderTextureTarget::Texture2d, equirectTextureId);

		RenderCommandData::SetTextureStorage2D textureStorage{
			RenderTextureTarget::Texture2d, 1, sizedFormat, equirectWidth, equirectHeight,
		};
		renderDevice->SetTextureStorage2D(&textureStorage);

		RenderCommandData::SetTextureSubImage2D textureImage{
			RenderTextureTarget::Texture2d, 0, 0, 0, equirectWidth, equirectHeight,
			RenderTextureBaseFormat::RGB, RenderTextureDataType::Float, equirectData
		};
		renderDevice->SetTextureSubImage2D(&textureImage);
	}

	{
		KOKKO_PROFILE_SCOPE("void stbi_image_free()");
		stbi_image_free(equirectData);
	}

	// Create result cubemap texture

	Vec2i envMapSize(EnvironmentTextureSize, EnvironmentTextureSize);
	TextureId envMapTextureId = textureManager->CreateTexture();
	textureManager->AllocateTextureStorage(envMapTextureId, RenderTextureTarget::TextureCubeMap, sizedFormat, 1, envMapSize);
	const TextureData& envMapTexture = textureManager->GetTextureData(envMapTextureId);

	renderDevice->BlendingDisable();
	renderDevice->ScissorTestDisable();
	renderDevice->BindSampler(0, samplerId);

	const MeshDrawData* cubeMeshDraw = meshManager->GetDrawData(cubeMeshId);

	// Load shader

	ShaderId equirectShaderId = shaderManager->FindShaderByPath(ConstStringView("engine/shaders/preprocess/equirect_to_cube.glsl"));
	const ShaderData& equirectShader = shaderManager->GetShaderData(equirectShaderId);

	// Bind common resources

	renderDevice->BindVertexArray(cubeMeshDraw->vertexArrayObject);

	RenderCommandData::BindFramebufferData bindFramebuffer{ RenderFramebufferTarget::Framebuffer, framebufferId };
	renderDevice->BindFramebuffer(&bindFramebuffer);

	// Bind shader

	renderDevice->UseShaderProgram(equirectShader.driverId);

	BindTexture(renderDevice, equirectShader, "equirectangular_map"_hash, RenderTextureTarget::Texture2d, equirectTextureId);

	// Render
	{
		KOKKO_PROFILE_SCOPE("Render equirect to cubemap");

		SetViewport(renderDevice, EnvironmentTextureSize);

		RenderCommandData::ClearMask clearMask{ true, false, false };

		for (unsigned int i = 0; i < CubemapSideCount; ++i)
		{
			BindBufferRange(renderDevice, UniformBlockBinding::Viewport,
				viewportUniformBufferId, viewportBlockStride * i, sizeof(ViewportUniformBlock));

			AttachFramebufferTexture(renderDevice, i, envMapTexture.textureObjectId, 0);

			renderDevice->DrawIndexed(cubeMeshDraw->primitiveMode, cubeMeshDraw->count, cubeMeshDraw->indexType);
		}
	}

	renderDevice->DestroyTextures(1, &equirectTextureId);
	equirectTextureId = 0;

	// Calculate diffuse irradiance

	// Create texture

	Vec2i diffuseMapSize(DiffuseTextureSize, DiffuseTextureSize);
	TextureId diffuseMapTextureId = textureManager->CreateTexture();
	textureManager->AllocateTextureStorage(diffuseMapTextureId, RenderTextureTarget::TextureCubeMap, sizedFormat, 1, diffuseMapSize);
	const TextureData& diffuseMapTexture = textureManager->GetTextureData(diffuseMapTextureId);

	// Load shader

	ShaderId calcDiffuseShaderId = shaderManager->FindShaderByPath(ConstStringView("engine/shaders/preprocess/calc_diffuse_irradiance.glsl"));
	const ShaderData& calcDiffuseShader = shaderManager->GetShaderData(calcDiffuseShaderId);

	renderDevice->UseShaderProgram(calcDiffuseShader.driverId);

	// Apply texture

	BindTexture(renderDevice, calcDiffuseShader, "environment_map"_hash, RenderTextureTarget::TextureCubeMap, envMapTexture.textureObjectId);

	{
		KOKKO_PROFILE_SCOPE("Convolute to diffuse irradiance map");

		SetViewport(renderDevice, DiffuseTextureSize);

		for (unsigned int i = 0; i < CubemapSideCount; ++i)
		{
			BindBufferRange(renderDevice, UniformBlockBinding::Viewport,
				viewportUniformBufferId, viewportBlockStride * i, sizeof(ViewportUniformBlock));

			AttachFramebufferTexture(renderDevice, i, diffuseMapTexture.textureObjectId, 0);

			renderDevice->DrawIndexed(cubeMeshDraw->primitiveMode, cubeMeshDraw->count, cubeMeshDraw->indexType);
		}
	}

	// Calculate specular irradiance

	// Create texture

	Vec2i specMapSize(SpecularTextureSize, SpecularTextureSize);
	TextureId specMapTextureId = textureManager->CreateTexture();
	textureManager->AllocateTextureStorage(specMapTextureId, RenderTextureTarget::TextureCubeMap,
		sizedFormat, SpecularMipmapLevelCount, specMapSize);

	const TextureData& specMapTexture = textureManager->GetTextureData(specMapTextureId);

	renderDevice->BindTexture(RenderTextureTarget::TextureCubeMap, specMapTexture.textureObjectId);
	renderDevice->SetTextureMinFilter(RenderTextureTarget::TextureCubeMap, RenderTextureFilterMode::LinearMipmap);
	renderDevice->SetTextureMagFilter(RenderTextureTarget::TextureCubeMap, RenderTextureFilterMode::Linear);

	// Load shader

	ShaderId calcSpecularShaderId = shaderManager->FindShaderByPath(ConstStringView("engine/shaders/preprocess/calc_specular_irradiance.glsl"));
	const ShaderData& calcSpecularShader = shaderManager->GetShaderData(calcSpecularShaderId);

	renderDevice->UseShaderProgram(calcSpecularShader.driverId);

	// Apply texture

	BindTexture(renderDevice, calcSpecularShader, "environment_map"_hash, RenderTextureTarget::TextureCubeMap, envMapTexture.textureObjectId);

	{
		KOKKO_PROFILE_SCOPE("Prefilter specular map");

		for (unsigned int mip = 0; mip < SpecularMipmapLevelCount; ++mip)
		{
			BindBufferRange(renderDevice, UniformBlockBinding::Object,
				specularUniformBufferId, specularBlockStride * mip, sizeof(CalcSpecularUniforms));

			// reisze framebuffer according to mip-level size.
			unsigned int mipSize = static_cast<unsigned int>(SpecularTextureSize) >> mip;

			SetViewport(renderDevice, static_cast<int>(mipSize));

			for (unsigned int i = 0; i < CubemapSideCount; ++i)
			{
				BindBufferRange(renderDevice, UniformBlockBinding::Viewport,
					viewportUniformBufferId, viewportBlockStride * i, sizeof(ViewportUniformBlock));

				AttachFramebufferTexture(renderDevice, i, specMapTexture.textureObjectId, mip);

				renderDevice->DrawIndexed(cubeMeshDraw->primitiveMode, cubeMeshDraw->count, cubeMeshDraw->indexType);
			}
		}
	}

	renderDevice->BindSampler(0, 0);
	
	EnvironmentComponent& environment = environmentMaps[id.i];

	environmentMaps[id.i].textures.environmentTexture = envMapTextureId;
	environmentMaps[id.i].textures.diffuseIrradianceTexture = diffuseMapTextureId;
	environmentMaps[id.i].textures.specularIrradianceTexture = specMapTextureId;
	environmentMaps[id.i].sourceTextureUid = textureUid;

	return;
}

EnvironmentId EnvironmentSystem::FindActiveEnvironment()
{
	for (size_t i = 1, count = environmentMaps.GetCount(); i < count; ++i)
	{
		if (environmentMaps[i].sourceTextureUid.HasValue())
			return EnvironmentId{ static_cast<unsigned int>(i) };
	}

	return EnvironmentId::Null;
}

EnvironmentTextures EnvironmentSystem::GetEnvironmentMap(EnvironmentId id) const
{
	assert(id != EnvironmentId::Null);
	return environmentMaps[id.i].textures;
}

Optional<Uid> EnvironmentSystem::GetSourceTextureUid(EnvironmentId id) const
{
	assert(id != EnvironmentId::Null);
	return environmentMaps[id.i].sourceTextureUid;
}

EnvironmentTextures EnvironmentSystem::GetEmptyEnvironmentMap() const
{
	return emptyEnvironmentMap;
}

void EnvironmentSystem::LoadEmptyEnvironmentMap()
{
	KOKKO_PROFILE_FUNCTION();

	static const int EmptyTextureSize = 1 << SpecularMipmapLevelCount;
	static const unsigned char EmptyTextureBytes[EmptyTextureSize * EmptyTextureSize * 3] = { 0 };

	RenderTextureSizedFormat sizedFormat = RenderTextureSizedFormat::RGB8;

	// Create environment texture

	Vec2i envMapSize(EmptyTextureSize, EmptyTextureSize);
	TextureId envMapTextureId = textureManager->CreateTexture();
	textureManager->AllocateTextureStorage(envMapTextureId, RenderTextureTarget::TextureCubeMap, sizedFormat, 1, envMapSize);
	const TextureData& envMapTexture = textureManager->GetTextureData(envMapTextureId);
	renderDevice->BindTexture(RenderTextureTarget::TextureCubeMap, envMapTexture.textureObjectId);

	for (unsigned int i = 0; i < CubemapSideCount; ++i)
	{
		RenderTextureTarget target = GetCubeTextureTarget(i);
		RenderCommandData::SetTextureSubImage2D subImageCommand{
			target, 0, 0, 0, EmptyTextureSize, EmptyTextureSize,
			RenderTextureBaseFormat::RGB, RenderTextureDataType::UnsignedByte, EmptyTextureBytes
		};
		renderDevice->SetTextureSubImage2D(&subImageCommand);
	}

	// Create diffuse irradiance texture

	Vec2i diffuseMapSize(EmptyTextureSize, EmptyTextureSize);
	TextureId diffuseMapTextureId = textureManager->CreateTexture();
	textureManager->AllocateTextureStorage(diffuseMapTextureId, RenderTextureTarget::TextureCubeMap, sizedFormat, 1, diffuseMapSize);
	const TextureData& diffuseMapTexture = textureManager->GetTextureData(diffuseMapTextureId);
	renderDevice->BindTexture(RenderTextureTarget::TextureCubeMap, diffuseMapTexture.textureObjectId);

	for (unsigned int i = 0; i < CubemapSideCount; ++i)
	{
		RenderTextureTarget target = GetCubeTextureTarget(i);
		RenderCommandData::SetTextureSubImage2D subImageCommand{
			target, 0, 0, 0, EmptyTextureSize, EmptyTextureSize,
			RenderTextureBaseFormat::RGB, RenderTextureDataType::UnsignedByte, EmptyTextureBytes
		};
		renderDevice->SetTextureSubImage2D(&subImageCommand);
	}

	// Create specular irradiance texture

	Vec2i specMapSize(EmptyTextureSize, EmptyTextureSize);
	TextureId specMapTextureId = textureManager->CreateTexture();
	textureManager->AllocateTextureStorage(specMapTextureId, RenderTextureTarget::TextureCubeMap,
		sizedFormat, SpecularMipmapLevelCount, specMapSize);
	const TextureData& specMapTexture = textureManager->GetTextureData(specMapTextureId);
	renderDevice->BindTexture(RenderTextureTarget::TextureCubeMap, specMapTexture.textureObjectId);

	for (int mip = 0; mip < SpecularMipmapLevelCount; ++mip)
	{
		int mipSize = EmptyTextureSize >> mip;

		for (unsigned int i = 0; i < CubemapSideCount; ++i)
		{
			RenderTextureTarget target = GetCubeTextureTarget(i);
			RenderCommandData::SetTextureSubImage2D subImageCommand{
				target, mip, 0, 0, mipSize, mipSize,
				RenderTextureBaseFormat::RGB, RenderTextureDataType::UnsignedByte, EmptyTextureBytes
			};
			renderDevice->SetTextureSubImage2D(&subImageCommand);
		}
	}

	emptyEnvironmentMap.environmentTexture = envMapTextureId;
	emptyEnvironmentMap.diffuseIrradianceTexture = diffuseMapTextureId;
	emptyEnvironmentMap.specularIrradianceTexture = specMapTextureId;
}

}
