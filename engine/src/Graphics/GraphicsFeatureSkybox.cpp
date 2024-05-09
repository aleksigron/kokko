#include "Graphics/GraphicsFeatureSkybox.hpp"

#include "Math/Mat4x4.hpp"

#include "Graphics/EnvironmentSystem.hpp"
#include "Graphics/GraphicsFeatureCommandList.hpp"

#include "Rendering/CommandEncoder.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderPassType.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/Uniform.hpp"

#include "Resources/ModelManager.hpp"
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
	alignas(4) float intensity;
};

} // Anynomous namespace

GraphicsFeatureSkybox::GraphicsFeatureSkybox() :
	shaderId(ShaderId::Null),
	meshId(ModelId::Null),
	uniformBufferId(0)
{
}

void GraphicsFeatureSkybox::Initialize(const InitializeParameters& parameters)
{
	kokko::render::Device* device = parameters.renderDevice;

	meshId = MeshPresets::CreateCube(parameters.modelManager);

	auto shaderPath = ConstStringView("engine/shaders/skybox/skybox.glsl");
	shaderId = parameters.shaderManager->FindShaderByPath(shaderPath);

	device->CreateBuffers(1, &uniformBufferId);
	device->SetBufferStorage(uniformBufferId, sizeof(SkyboxUniformBlock), nullptr, BufferStorageFlags::Dynamic);
}

void GraphicsFeatureSkybox::Deinitialize(const InitializeParameters& parameters)
{
	parameters.renderDevice->DestroyBuffers(1, &uniformBufferId);
	uniformBufferId = render::BufferId();

	parameters.modelManager->RemoveModel(meshId);
	meshId = ModelId::Null;
}

void GraphicsFeatureSkybox::Upload(const UploadParameters& parameters)
{
	kokko::render::Device* device = parameters.renderDevice;
	EnvironmentSystem* envSystem = parameters.environmentSystem;

	Mat4x4f cameraProjection = parameters.cameraParameters.projection.GetProjectionMatrix(true);
	const auto& cameraTransforms = parameters.cameraParameters.transform;

	Vec3f cameraPos = (cameraTransforms.forward * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

	SkyboxUniformBlock skyboxUniforms;
	skyboxUniforms.transform = cameraProjection * cameraTransforms.inverse * Mat4x4f::Translate(cameraPos);
	skyboxUniforms.intensity = 1.0f;

	EnvironmentId envId = envSystem->FindActiveEnvironment();
	if (envId != EnvironmentId::Null)
		skyboxUniforms.intensity = envSystem->GetIntensity(envId);

	device->SetBufferSubData(uniformBufferId, 0, sizeof(SkyboxUniformBlock), &skyboxUniforms);
}

void GraphicsFeatureSkybox::Submit(const SubmitParameters& parameters)
{
	parameters.commandList.AddToFullscreenViewport(RenderPassType::Skybox, 0.0f, 0);
}

void GraphicsFeatureSkybox::Render(const RenderParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	render::CommandEncoder* encoder = parameters.encoder;

	kokko::EnvironmentTextures envMap;
	kokko::EnvironmentId envId = parameters.environmentSystem->FindActiveEnvironment();
	if (envId != kokko::EnvironmentId::Null)
		envMap = parameters.environmentSystem->GetEnvironmentMap(envId);
	else
		envMap = parameters.environmentSystem->GetEmptyEnvironmentMap();

	const TextureData& envTexture = parameters.textureManager->GetTextureData(envMap.environmentTexture);

	const ShaderData& shader = parameters.shaderManager->GetShaderData(shaderId);
	encoder->UseShaderProgram(shader.driverId);

	const kokko::TextureUniform* uniform = shader.uniforms.FindTextureUniformByNameHash("environment_map"_hash);
	if (uniform != nullptr)
	{
		encoder->BindTextureToShader(uniform->uniformLocation, 0, envTexture.textureObjectId);
	}

	encoder->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, uniformBufferId);

	auto& mesh = parameters.modelManager->GetModelMeshes(meshId)[0];
	auto& prim = parameters.modelManager->GetModelPrimitives(meshId)[0];
	encoder->BindVertexArray(prim.vertexArrayId);
	encoder->DrawIndexed(mesh.primitiveMode, mesh.indexType, prim.count, 0, 0);
}

}
