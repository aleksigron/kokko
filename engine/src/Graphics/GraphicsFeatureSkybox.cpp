#include "Graphics/GraphicsFeatureSkybox.hpp"

#include "Math/Mat4x4.hpp"

#include "Graphics/EnvironmentSystem.hpp"
#include "Graphics/GraphicsFeatureCommandList.hpp"

#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderPassType.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/Uniform.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"

namespace kokko
{

namespace
{
struct SkyboxUniformBlock
{
	alignas(16) Mat4x4f transform;
};

} // Anynomous namespace

GraphicsFeatureSkybox::GraphicsFeatureSkybox() :
	shaderId(ShaderId::Null),
	meshId(MeshId::Null),
	uniformBufferId(0)
{
}

void GraphicsFeatureSkybox::Initialize(const InitializeParameters& parameters)
{
	RenderDevice* device = parameters.renderDevice;

	meshId = parameters.meshManager->CreateMesh();
	MeshPresets::UploadCube(parameters.meshManager, meshId);

	auto shaderPath = ConstStringView("engine/shaders/skybox/skybox.glsl");
	shaderId = parameters.shaderManager->FindShaderByPath(shaderPath);

	device->CreateBuffers(1, &uniformBufferId);
	device->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferId);

	RenderCommandData::SetBufferStorage storage{};
	storage.target = RenderBufferTarget::UniformBuffer;
	storage.size = sizeof(SkyboxUniformBlock);
	storage.data = nullptr;
	storage.dynamicStorage = true;
	device->SetBufferStorage(&storage);
}

void GraphicsFeatureSkybox::Deinitialize(const InitializeParameters& parameters)
{
	parameters.renderDevice->DestroyBuffers(1, &uniformBufferId);
	uniformBufferId = 0;

	parameters.meshManager->RemoveMesh(meshId);
	meshId = MeshId::Null;
}

void GraphicsFeatureSkybox::Upload(const UploadParameters& parameters)
{
	RenderDevice* device = parameters.renderDevice;

	Mat4x4f cameraProjection = parameters.cameraParameters.projection.GetProjectionMatrix(true);
	const auto& cameraTransforms = parameters.cameraParameters.transform;

	Vec3f cameraPos = (cameraTransforms.forward * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

	SkyboxUniformBlock skyboxUniforms;
	skyboxUniforms.transform = cameraProjection * cameraTransforms.inverse * Mat4x4f::Translate(cameraPos);

	device->BindBuffer(RenderBufferTarget::UniformBuffer, uniformBufferId);
	device->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(SkyboxUniformBlock), &skyboxUniforms);
}

void GraphicsFeatureSkybox::Submit(const SubmitParameters& parameters)
{
	parameters.commandList.AddToFullscreenViewport(RenderPassType::Skybox, 0.0f, 0);
}

void GraphicsFeatureSkybox::Render(const RenderParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	RenderDevice* device = parameters.renderDevice;

	kokko::EnvironmentTextures envMap;
	kokko::EnvironmentId envId = parameters.environmentSystem->FindActiveEnvironment();
	if (envId != kokko::EnvironmentId::Null)
		envMap = parameters.environmentSystem->GetEnvironmentMap(envId);
	else
		envMap = parameters.environmentSystem->GetEmptyEnvironmentMap();

	const TextureData& envTexture = parameters.textureManager->GetTextureData(envMap.environmentTexture);

	const ShaderData& shader = parameters.shaderManager->GetShaderData(shaderId);
	device->UseShaderProgram(shader.driverId);

	const kokko::TextureUniform* uniform = shader.uniforms.FindTextureUniformByNameHash("environment_map"_hash);
	if (uniform != nullptr)
	{
		device->SetActiveTextureUnit(0);
		device->BindTexture(envTexture.textureTarget, envTexture.textureObjectId);
		device->SetUniformInt(uniform->uniformLocation, 0);
	}

	device->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, uniformBufferId);

	const MeshDrawData* draw = parameters.meshManager->GetDrawData(meshId);
	device->BindVertexArray(draw->vertexArrayObject);

	device->DrawIndexed(draw->primitiveMode, draw->count, draw->indexType);
}

}
