#include "ParticleSystem.hpp"

#include <cstring>
#include <random>

#include "Math/Vec3.hpp"

#include "Rendering/RenderCommandList.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/RenderViewport.hpp"
#include "Rendering/StaticUniformBuffer.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ShaderManager.hpp"

#include "System/Time.hpp"

struct UpdateParticleBlock
{
	alignas(16) Vec3f gravity;
	alignas(4) float deltaTime;
};

ParticleSystem::ParticleSystem(
	RenderDevice* renderDevice,
	ShaderManager* shaderManager,
	MeshManager* meshManager) :
	renderDevice(renderDevice),
	shaderManager(shaderManager),
	meshManager(meshManager),
	quadMeshId(MeshId{ 0 }),
	updateShaderId(ShaderId{ 0 }),
	renderShaderId(ShaderId{ 0 }),
	customRenderCallback(0),
	particleCount(0)
{
	for (unsigned int i = 0; i < BufferCount; ++i)
		bufferIds[i] = 0;
}

ParticleSystem::~ParticleSystem()
{
	if (bufferIds[0] != 0)
	{
		renderDevice->DestroyBuffers(BufferCount, bufferIds);

		for (unsigned int i = 0; i < BufferCount; ++i)
			bufferIds[i] = 0;
	}
}

void ParticleSystem::Initialize(Renderer* renderer)
{
	customRenderCallback = renderer->AddCustomRenderer(this);

	quadMeshId = meshManager->CreateMesh();
	MeshPresets::UploadPlane(meshManager, quadMeshId);

	updateShaderId = shaderManager->GetIdByPath(StringRef("res/shaders/particles/particle_update.shader.json"));
	renderShaderId = shaderManager->GetIdByPath(StringRef("res/shaders/particles/particle_render.shader.json"));

	static const size_t Vec3Size = sizeof(float) * 3;
	static const size_t Vec4Size = sizeof(float) * 4;

	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	std::mt19937 generator;

	renderDevice->CreateBuffers(BufferCount, bufferIds);

	// Position buffer

	size_t posBufferSize = Vec3Size * MaxParticleCount;

	//float buffer[MaxParticleCount * 4];

	renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, bufferIds[Buffer_Position]);

	/*
	for (size_t i = 0; i < MaxParticleCount; ++i)
	{
		buffer[i * 4 + 0] = distribution(generator) * 4.0f - 2.0f;
		buffer[i * 4 + 1] = distribution(generator) * 2.0f + 1.0f;
		buffer[i * 4 + 2] = distribution(generator) * 4.0f - 2.0f;
		buffer[i * 4 + 3] = 0.0f;
	}

	renderDevice->SetBufferData(RenderBufferTarget::ShaderStorageBuffer, sizeof(buffer), buffer, RenderBufferUsage::DynamicDraw);
	*/

	renderDevice->SetBufferData(RenderBufferTarget::ShaderStorageBuffer, posBufferSize, nullptr, RenderBufferUsage::DynamicDraw);

	RenderCommandData::MapBufferRange mapPosBuffer{};
	mapPosBuffer.target = RenderBufferTarget::ShaderStorageBuffer;
	mapPosBuffer.offset = 0;
	mapPosBuffer.length = posBufferSize;
	mapPosBuffer.writeAccess = true;
	mapPosBuffer.invalidateBuffer = true;

	float* posBuffer = static_cast<float*>(renderDevice->MapBufferRange(&mapPosBuffer));

	for (size_t i = 0; i < MaxParticleCount; ++i)
	{
		posBuffer[i * 4 + 0] = distribution(generator) * 4.0f - 2.0f;
		posBuffer[i * 4 + 1] = distribution(generator) * 2.0f + 1.0f;
		posBuffer[i * 4 + 2] = distribution(generator) * 4.0f - 2.0f;
		posBuffer[i * 4 + 3] = 0.0f;
	}

	renderDevice->UnmapBuffer(RenderBufferTarget::ShaderStorageBuffer);

	// Velocity buffer

	size_t velBufferSize = Vec4Size * MaxParticleCount;

	renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, bufferIds[Buffer_Velocity]);
	renderDevice->SetBufferData(RenderBufferTarget::ShaderStorageBuffer, velBufferSize, nullptr, RenderBufferUsage::DynamicDraw);

	RenderCommandData::MapBufferRange mapVelBuffer{};
	mapVelBuffer.target = RenderBufferTarget::ShaderStorageBuffer;
	mapVelBuffer.offset = 0;
	mapVelBuffer.length = velBufferSize;
	mapVelBuffer.writeAccess = true;
	mapVelBuffer.invalidateBuffer = true;

	float* velBuffer = static_cast<float*>(renderDevice->MapBufferRange(&mapVelBuffer));
	for (size_t i = 0; i < MaxParticleCount; ++i)
	{
		velBuffer[i * 4 + 0] = distribution(generator) * 0.5f - 0.25f;
		velBuffer[i * 4 + 1] = distribution(generator) * 1.0f + 3.0f;
		velBuffer[i * 4 + 2] = distribution(generator) * 0.5f - 0.25f;
		velBuffer[i * 4 + 3] = 0.0f;
	}

	renderDevice->UnmapBuffer(RenderBufferTarget::ShaderStorageBuffer);

	// Update uniform buffer

	size_t upSize = sizeof(UpdateParticleBlock);
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, bufferIds[Buffer_UpdateUniforms]);
	renderDevice->SetBufferData(RenderBufferTarget::UniformBuffer, upSize, nullptr, RenderBufferUsage::DynamicDraw);

	// Object uniform buffer

	size_t obSize = sizeof(TransformUniformBlock);
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, bufferIds[Buffer_RenderTransform]);
	renderDevice->SetBufferData(RenderBufferTarget::UniformBuffer, obSize, nullptr, RenderBufferUsage::DynamicDraw);
}

void ParticleSystem::AddRenderCommands(const CommandParams& params)
{
	float depth = 0.0f; // TODO: Calculate
	params.commandList->AddDrawWithCallback(params.fullscreenViewport, RenderPass::Transparent, depth, params.callbackId);
}

void ParticleSystem::RenderCustom(const RenderParams& params)
{
	UpdateParticleBlock updateUniforms;
	updateUniforms.gravity = Vec3f(0.0f, -9.82f, 0.0f);
	updateUniforms.deltaTime = Time::GetDeltaTime();

	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, bufferIds[Buffer_UpdateUniforms]);
	renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(UpdateParticleBlock), &updateUniforms);

	TransformUniformBlock transformUniforms;
	transformUniforms.M = Mat4x4f();
	transformUniforms.MV = params.viewport->view;
	transformUniforms.MVP = params.viewport->viewProjection;

	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, bufferIds[Buffer_RenderTransform]);
	renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(TransformUniformBlock), &transformUniforms);

	const MeshDrawData* draw = meshManager->GetDrawData(quadMeshId);

	particleCount = MaxParticleCount;
	unsigned int groupCount = static_cast<unsigned int>(particleCount / ParticlesPerWorkgroup);

	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 0, bufferIds[Buffer_Position]);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 1, bufferIds[Buffer_Velocity]);
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, bufferIds[Buffer_UpdateUniforms]);

	const ShaderData& updateShader = shaderManager->GetShaderData(updateShaderId);
	renderDevice->UseShaderProgram(updateShader.driverId);
	renderDevice->DispatchCompute(groupCount, 1, 1);

	// Make sure the buffer particle buffer updates are visible to the draw command
	RenderCommandData::MemoryBarrier shaderStorageBarrier{};
	shaderStorageBarrier.shaderStorage = true;
	renderDevice->MemoryBarrier(shaderStorageBarrier);

	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Viewport, params.viewport->uniformBlockObject);
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, bufferIds[Buffer_RenderTransform]);

	const ShaderData& renderShader = shaderManager->GetShaderData(renderShaderId);
	renderDevice->UseShaderProgram(renderShader.driverId);
	renderDevice->DrawIndexedInstanced(draw->primitiveMode, draw->count, draw->indexType, particleCount);
}
