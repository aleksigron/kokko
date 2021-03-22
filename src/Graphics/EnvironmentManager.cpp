#include "EnvironmentManager.hpp"

#include "stb_image/stb_image.h"

#include "Core/Buffer.hpp"

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
	environmentMaps(allocator)
{
}

EnvironmentManager::~EnvironmentManager()
{
}

int EnvironmentManager::LoadHdrEnvironmentMap(const char* equirectMapPath)
{
	static const int EnvironmentTextureSize = 1024;
	static const int IrradianceTextureSize = 64;
	static const size_t CubemapSides = 6;

	RenderTextureSizedFormat sizedFormat = RenderTextureSizedFormat::RGB16F;

	// Load equirectangular image and create texture

	stbi_set_flip_vertically_on_load(true);
	int equirectWidth, equirectHeight, nrComponents;
	float* equirectData = stbi_loadf(equirectMapPath, &equirectWidth, &equirectHeight, &nrComponents, 0);

	unsigned int equirectTextureId = 0;

	if (equirectData)
	{
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

		stbi_image_free(equirectData);
	}
	else
	{
		Log::Error("Couldn't load HDR texture file");

		return -1;
	}

	// Create framebuffer

	unsigned int framebuffer;
	renderDevice->CreateFramebuffers(1, &framebuffer);

	// Create result cubemap texture

	Vec2i envMapSize(EnvironmentTextureSize, EnvironmentTextureSize);
	TextureId envMapTextureId = textureManager->CreateTexture();
	textureManager->AllocateTextureStorage(envMapTextureId, RenderTextureTarget::TextureCubeMap, sizedFormat, 1, envMapSize);
	const TextureData& envMapTexture = textureManager->GetTextureData(envMapTextureId);

	// Create sampler object

	unsigned int sampler;
	renderDevice->CreateSamplers(1, &sampler);

	RenderCommandData::SetSamplerParameters samplerParams{
		sampler, RenderTextureFilterMode::Linear, RenderTextureFilterMode::Linear,
		RenderTextureWrapMode::ClampToEdge, RenderTextureWrapMode::ClampToEdge, RenderTextureWrapMode::ClampToEdge,
		RenderTextureCompareMode::None
	};
	renderDevice->SetSamplerParameters(&samplerParams);

	renderDevice->BindSampler(0, sampler);

	// Create viewport uniform buffer

	ProjectionParameters projection;
	projection.near = 0.1f;
	projection.far = 1.0f;
	projection.aspect = 1.0f;
	projection.SetPerspective(Math::DegreesToRadians(90.0f));
	Mat4x4f projectionMatrix = projection.GetProjectionMatrix(false);

	Vec3f zero3(0.0f, 0.0f, 0.0f);
	Mat4x4f viewTransforms[CubemapSides] = {
		Camera::GetViewMatrix(Mat4x4f::LookAt(zero3, Vec3f(1.0f, 0.0f, 0.0f), Vec3f(0.0f, -1.0f, 0.0f))),
		Camera::GetViewMatrix(Mat4x4f::LookAt(zero3, Vec3f(-1.0f, 0.0f, 0.0f), Vec3f(0.0f, -1.0f, 0.0f))),
		Camera::GetViewMatrix(Mat4x4f::LookAt(zero3, Vec3f(0.0f, 1.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f))),
		Camera::GetViewMatrix(Mat4x4f::LookAt(zero3, Vec3f(0.0f, -1.0f, 0.0f), Vec3f(0.0f, 0.0f, -1.0f))),
		Camera::GetViewMatrix(Mat4x4f::LookAt(zero3, Vec3f(0.0f, 0.0f, 1.0f), Vec3f(0.0f, -1.0f, 0.0f))),
		Camera::GetViewMatrix(Mat4x4f::LookAt(zero3, Vec3f(0.0f, 0.0f, -1.0f), Vec3f(0.0f, -1.0f, 0.0f))),
	};

	// TODO: Refactor into a reusable function
	RenderTextureTarget cubeTextureTargets[CubemapSides] = {
		RenderTextureTarget::TextureCubeMap_PositiveX,
		RenderTextureTarget::TextureCubeMap_NegativeX,
		RenderTextureTarget::TextureCubeMap_PositiveY,
		RenderTextureTarget::TextureCubeMap_NegativeY,
		RenderTextureTarget::TextureCubeMap_PositiveZ,
		RenderTextureTarget::TextureCubeMap_NegativeZ
	};

	unsigned int viewportUniformBuffer;
	renderDevice->CreateBuffers(1, &viewportUniformBuffer);
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, viewportUniformBuffer);

	int alignment;
	renderDevice->GetIntegerValue(RenderDeviceParameter::UniformBufferOffsetAlignment, &alignment);

	size_t blockStride = (sizeof(TransformUniformBlock) + alignment - 1) / alignment * alignment;
	unsigned int viewportBufferSize = static_cast<unsigned int>(blockStride * CubemapSides);

	Buffer<unsigned char> uniformBuffer(allocator);
	uniformBuffer.Allocate(viewportBufferSize);

	for (size_t i = 0; i < CubemapSides; ++i)
	{
		ViewportUniformBlock* uniforms = reinterpret_cast<ViewportUniformBlock*>(&uniformBuffer[blockStride * i]);

		uniforms->V = viewTransforms[i];
		uniforms->P = projectionMatrix;
		uniforms->VP = projectionMatrix * viewTransforms[i];
	}

	RenderCommandData::SetBufferStorage bufferStorage{
		RenderBufferTarget::UniformBuffer, viewportBufferSize, uniformBuffer.Data()
	};
	renderDevice->SetBufferStorage(&bufferStorage);

	uniformBuffer.Deallocate();

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

	RenderCommandData::ViewportData envViewport{
		0, 0, EnvironmentTextureSize, EnvironmentTextureSize
	};
	renderDevice->Viewport(&envViewport);

	RenderCommandData::BindFramebufferData bindFramebuffer{
		RenderFramebufferTarget::Framebuffer, framebuffer
	};
	renderDevice->BindFramebuffer(&bindFramebuffer);

	renderDevice->BindVertexArray(cubeMeshDraw->vertexArrayObject);

	RenderCommandData::ClearMask clearMask{ true, false, false };

	for (unsigned int i = 0; i < CubemapSides; ++i)
	{
		RenderCommandData::BindBufferRange bindBufferRange{
			RenderBufferTarget::UniformBuffer, UniformBlockBinding::Viewport, viewportUniformBuffer,
			blockStride * i, sizeof(ViewportUniformBlock)
		};
		renderDevice->BindBufferRange(&bindBufferRange);

		RenderCommandData::AttachFramebufferTexture2D attachFramebufferTexture{
			RenderFramebufferTarget::Framebuffer, RenderFramebufferAttachment::Color0,
			cubeTextureTargets[i], envMapTexture.textureObjectId, 0
		};
		renderDevice->AttachFramebufferTexture2D(&attachFramebufferTexture);

		renderDevice->DrawIndexed(cubeMeshDraw->primitiveMode, cubeMeshDraw->count, cubeMeshDraw->indexType);
	}

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

	RenderCommandData::ViewportData irrViewport{
		0, 0, IrradianceTextureSize, IrradianceTextureSize
	};
	renderDevice->Viewport(&irrViewport);

	for (unsigned int i = 0; i < CubemapSides; ++i)
	{
		RenderCommandData::BindBufferRange bindBufferRange{
			RenderBufferTarget::UniformBuffer, UniformBlockBinding::Viewport, viewportUniformBuffer,
			blockStride * i, sizeof(ViewportUniformBlock)
		};
		renderDevice->BindBufferRange(&bindBufferRange);

		RenderCommandData::AttachFramebufferTexture2D attachFramebufferTexture{
			RenderFramebufferTarget::Framebuffer, RenderFramebufferAttachment::Color0,
			cubeTextureTargets[i], irrMapTexture.textureObjectId, 0
		};
		renderDevice->AttachFramebufferTexture2D(&attachFramebufferTexture);

		renderDevice->DrawIndexed(cubeMeshDraw->primitiveMode, cubeMeshDraw->count, cubeMeshDraw->indexType);
	}

	renderDevice->BindSampler(0, 0);

	renderDevice->DestroyTextures(1, &equirectTextureId);
	renderDevice->DestroyBuffers(1, &viewportUniformBuffer);
	renderDevice->DestroySamplers(1, &sampler);
	renderDevice->DestroyFramebuffers(1, &framebuffer);

	unsigned int envIndex = environmentMaps.GetCount();
	environmentMaps.PushBack(EnvironmentTextures{ envMapTextureId, irrMapTextureId });

	return static_cast<int>(envIndex);
}
