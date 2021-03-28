#include "EnvironmentManager.hpp"

#include "stb_image/stb_image.h"

#include "Core/Buffer.hpp"
#include "Core/Core.hpp"

#include "Debug/LogHelper.hpp"

#include "Math/Mat4x4.hpp"
#include "Math/Math.hpp"
#include "Math/Projection.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/Camera.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/StaticUniformBuffer.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"

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

EnvironmentManager::EnvironmentManager(
	Allocator* allocator,
	RenderDevice* renderDevice,
	ShaderManager* shaderManager,
	MeshManager* meshManager,
	TextureManager* textureManager) :
	allocator(allocator),
	renderDevice(renderDevice),
	shaderManager(shaderManager),
	meshManager(meshManager),
	textureManager(textureManager),
	environmentMaps(allocator),
	blockStride(0),
	framebufferId(0),
	viewportUniformBufferId(0),
	samplerId(0)
{
}

EnvironmentManager::~EnvironmentManager()
{
	Deinitialize();
}

void EnvironmentManager::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	// Create framebuffer

	renderDevice->CreateFramebuffers(1, &framebufferId);

	// Create viewport uniform buffer

	ProjectionParameters projection;
	projection.near = 0.1f;
	projection.far = 1.0f;
	projection.aspect = 1.0f;
	projection.SetPerspective(Math::DegreesToRadians(90.0f));
	Mat4x4f projectionMatrix = projection.GetProjectionMatrix(false);

	Vec3f zero3(0.0f, 0.0f, 0.0f);
	Mat4x4f viewTransforms[CubemapSideCount] = {
		Camera::GetViewMatrix(Mat4x4f::LookAt(zero3, Vec3f(1.0f, 0.0f, 0.0f), Vec3f(0.0f, -1.0f, 0.0f))),
		Camera::GetViewMatrix(Mat4x4f::LookAt(zero3, Vec3f(-1.0f, 0.0f, 0.0f), Vec3f(0.0f, -1.0f, 0.0f))),
		Camera::GetViewMatrix(Mat4x4f::LookAt(zero3, Vec3f(0.0f, 1.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f))),
		Camera::GetViewMatrix(Mat4x4f::LookAt(zero3, Vec3f(0.0f, -1.0f, 0.0f), Vec3f(0.0f, 0.0f, -1.0f))),
		Camera::GetViewMatrix(Mat4x4f::LookAt(zero3, Vec3f(0.0f, 0.0f, 1.0f), Vec3f(0.0f, -1.0f, 0.0f))),
		Camera::GetViewMatrix(Mat4x4f::LookAt(zero3, Vec3f(0.0f, 0.0f, -1.0f), Vec3f(0.0f, -1.0f, 0.0f))),
	};

	int alignment;
	renderDevice->GetIntegerValue(RenderDeviceParameter::UniformBufferOffsetAlignment, &alignment);

	blockStride = (sizeof(TransformUniformBlock) + alignment - 1) / alignment * alignment;
	unsigned int viewportBufferSize = static_cast<unsigned int>(blockStride * CubemapSideCount);

	Buffer<unsigned char> uniformBuffer(allocator);
	uniformBuffer.Allocate(viewportBufferSize);

	for (size_t i = 0; i < CubemapSideCount; ++i)
	{
		ViewportUniformBlock* uniforms = reinterpret_cast<ViewportUniformBlock*>(&uniformBuffer[blockStride * i]);

		uniforms->V = viewTransforms[i];
		uniforms->P = projectionMatrix;
		uniforms->VP = projectionMatrix * viewTransforms[i];
	}

	renderDevice->CreateBuffers(1, &viewportUniformBufferId);
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, viewportUniformBufferId);

	RenderCommandData::SetBufferStorage bufferStorage{
		RenderBufferTarget::UniformBuffer, viewportBufferSize, uniformBuffer.Data()
	};
	renderDevice->SetBufferStorage(&bufferStorage);

	uniformBuffer.Deallocate();

	// Create sampler object

	renderDevice->CreateSamplers(1, &samplerId);
	RenderCommandData::SetSamplerParameters samplerParams{
		samplerId, RenderTextureFilterMode::Linear, RenderTextureFilterMode::Linear,
		RenderTextureWrapMode::ClampToEdge, RenderTextureWrapMode::ClampToEdge, RenderTextureWrapMode::ClampToEdge,
		RenderTextureCompareMode::None
	};
	renderDevice->SetSamplerParameters(&samplerParams);
}

void EnvironmentManager::Deinitialize()
{
	if (samplerId != 0)
	{
		renderDevice->DestroySamplers(1, &samplerId);
		samplerId = 0;
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
}

int EnvironmentManager::LoadHdrEnvironmentMap(const char* equirectMapPath)
{
	KOKKO_PROFILE_FUNCTION();

	static const int EnvironmentTextureSize = 1024;
	static const int IrradianceTextureSize = 32;

	RenderTextureSizedFormat sizedFormat = RenderTextureSizedFormat::RGB16F;

	// Load equirectangular image and create texture

	stbi_set_flip_vertically_on_load(true);
	int equirectWidth, equirectHeight, nrComponents;
	float* equirectData;
	{
		KOKKO_PROFILE_SCOPE("void* stbi_loadf()");
		equirectData = stbi_loadf(equirectMapPath, &equirectWidth, &equirectHeight, &nrComponents, 0);
	}

	unsigned int equirectTextureId = 0;

	if (equirectData)
	{
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
	}
	else
	{
		Log::Error("Couldn't load HDR texture file");

		return -1;
	}

	// Create result cubemap texture

	Vec2i envMapSize(EnvironmentTextureSize, EnvironmentTextureSize);
	TextureId envMapTextureId = textureManager->CreateTexture();
	textureManager->AllocateTextureStorage(envMapTextureId, RenderTextureTarget::TextureCubeMap, sizedFormat, 1, envMapSize);
	const TextureData& envMapTexture = textureManager->GetTextureData(envMapTextureId);

	renderDevice->BindSampler(0, samplerId);

	// Load cube mesh

	MeshId cubeMesh = meshManager->CreateMesh();
	MeshPresets::UploadCube(meshManager, cubeMesh);
	const MeshDrawData* cubeMeshDraw = meshManager->GetDrawData(cubeMesh);

	// Load shader

	ShaderId equirectShaderId = shaderManager->GetIdByPath(StringRef("res/shaders/preprocess/equirect_to_cube.shader.json"));
	const ShaderData& equirectShader = shaderManager->GetShaderData(equirectShaderId);

	renderDevice->UseShaderProgram(equirectShader.driverId);

	// Apply texture

	const TextureUniform* eqTU = equirectShader.uniforms.FindTextureUniformByNameHash("equirectangular_map"_hash);
	if (eqTU != nullptr)
	{
		renderDevice->SetUniformInt(eqTU->uniformLocation, 0);
		renderDevice->SetActiveTextureUnit(0);
		renderDevice->BindTexture(RenderTextureTarget::Texture2d, equirectTextureId);
	}

	// Render
	{
		KOKKO_PROFILE_SCOPE("Render equirect to cubemap");

		RenderCommandData::ViewportData envViewport{
			0, 0, EnvironmentTextureSize, EnvironmentTextureSize
		};
		renderDevice->Viewport(&envViewport);

		RenderCommandData::BindFramebufferData bindFramebuffer{ RenderFramebufferTarget::Framebuffer, framebufferId };
		renderDevice->BindFramebuffer(&bindFramebuffer);

		renderDevice->BindVertexArray(cubeMeshDraw->vertexArrayObject);

		RenderCommandData::ClearMask clearMask{ true, false, false };

		for (unsigned int i = 0; i < CubemapSideCount; ++i)
		{
			RenderCommandData::BindBufferRange bindBufferRange{
				RenderBufferTarget::UniformBuffer, UniformBlockBinding::Viewport, viewportUniformBufferId,
				blockStride * i, sizeof(ViewportUniformBlock)
			};
			renderDevice->BindBufferRange(&bindBufferRange);

			RenderCommandData::AttachFramebufferTexture2D attachFramebufferTexture{
				RenderFramebufferTarget::Framebuffer, RenderFramebufferAttachment::Color0,
				GetCubeTextureTarget(i), envMapTexture.textureObjectId, 0
			};
			renderDevice->AttachFramebufferTexture2D(&attachFramebufferTexture);

			renderDevice->DrawIndexed(cubeMeshDraw->primitiveMode, cubeMeshDraw->count, cubeMeshDraw->indexType);
		}
	}

	renderDevice->DestroyTextures(1, &equirectTextureId);
	equirectTextureId = 0;

	// Now we have the environment map in a cubemap format
	// Next we need to calculate the diffuse irradiance in each direction

	// Create texture

	Vec2i irrMapSize(IrradianceTextureSize, IrradianceTextureSize);
	TextureId irrMapTextureId = textureManager->CreateTexture();
	textureManager->AllocateTextureStorage(irrMapTextureId, RenderTextureTarget::TextureCubeMap, sizedFormat, 1, irrMapSize);
	const TextureData& irrMapTexture = textureManager->GetTextureData(irrMapTextureId);

	// Load shader

	ShaderId calcIrradianceShaderId = shaderManager->GetIdByPath(StringRef("res/shaders/preprocess/calc_diffuse_irradiance.shader.json"));
	const ShaderData& calcIrradianceShader = shaderManager->GetShaderData(calcIrradianceShaderId);

	renderDevice->UseShaderProgram(calcIrradianceShader.driverId);

	// Apply texture

	const TextureUniform* envTU = calcIrradianceShader.uniforms.FindTextureUniformByNameHash("environment_map"_hash);
	if (envTU != nullptr)
	{
		renderDevice->SetUniformInt(envTU->uniformLocation, 0);
		renderDevice->SetActiveTextureUnit(0);
		renderDevice->BindTexture(RenderTextureTarget::TextureCubeMap, envMapTexture.textureObjectId);
	}

	{

		KOKKO_PROFILE_SCOPE("Convolute to diffuse irradiance map");

		RenderCommandData::ViewportData irrViewport{
			0, 0, IrradianceTextureSize, IrradianceTextureSize
		};
		renderDevice->Viewport(&irrViewport);

		for (unsigned int i = 0; i < CubemapSideCount; ++i)
		{
			RenderCommandData::BindBufferRange bindBufferRange{
				RenderBufferTarget::UniformBuffer, UniformBlockBinding::Viewport, viewportUniformBufferId,
				blockStride * i, sizeof(ViewportUniformBlock)
			};
			renderDevice->BindBufferRange(&bindBufferRange);

			RenderCommandData::AttachFramebufferTexture2D attachFramebufferTexture{
				RenderFramebufferTarget::Framebuffer, RenderFramebufferAttachment::Color0,
				GetCubeTextureTarget(i), irrMapTexture.textureObjectId, 0
			};
			renderDevice->AttachFramebufferTexture2D(&attachFramebufferTexture);

			renderDevice->DrawIndexed(cubeMeshDraw->primitiveMode, cubeMeshDraw->count, cubeMeshDraw->indexType);
		}
	}

	renderDevice->BindSampler(0, 0);

	unsigned int envIndex = environmentMaps.GetCount();
	environmentMaps.PushBack(EnvironmentTextures{ envMapTextureId, irrMapTextureId });

	return static_cast<int>(envIndex);
}
