#include "ParticleSystem.hpp"

#include <cassert>
#include <cstring>

#include "Core/Core.hpp"

#include "Engine/Entity.hpp"

#include "Math/Vec3.hpp"
#include "Math/Random.hpp"

#include "Graphics/GraphicsFeatureCommandList.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/RenderViewport.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/Uniform.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ShaderManager.hpp"

#include "System/Time.hpp"

namespace kokko
{

const ParticleEmitterId ParticleEmitterId::Null = ParticleEmitterId{ 0 };

struct UpdateParticleBlock
{
	alignas(16) Vec3f emitPosition;
	alignas(4) float emitPositionVariance;
	alignas(16) Vec3f initialVelocity;
	alignas(4) float initialVelocityVariance;
	alignas(16) Vec3f gravity;
	alignas(4) int noiseTextureSize;
	alignas(4) int noiseSeed;
	alignas(4) int emitCount;
	alignas(4) float particleLifetime;
	alignas(4) float particleSize;
	alignas(4) float bounceEnergy;
	alignas(4) float currentTime;
	alignas(4) float deltaTime;
};

ParticleSystem::ParticleSystem(
	Allocator* allocator,
	RenderDevice* renderDevice,
	ShaderManager* shaderManager,
	MeshManager* meshManager) :
	allocator(allocator),
	renderDevice(renderDevice),
	shaderManager(shaderManager),
	meshManager(meshManager),
	quadMeshId(MeshId{ 0 }),
	initUpdateShaderId(ShaderId{ 0 }),
	emitShaderId(ShaderId{ 0 }),
	simulateShaderId(ShaderId{ 0 }),
	finishUpdateShaderId(ShaderId{ 0 }),
	renderShaderId(ShaderId{ 0 }),
	noiseTextureId(0),
	entityMap(allocator)
{
	data = InstanceData{};
}

ParticleSystem::~ParticleSystem()
{
	Deinitialize();
}

void ParticleSystem::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	float vertexBuffer[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};

	VertexAttribute vertexAttr[] = { VertexAttribute::pos3 };
	VertexFormat format(vertexAttr, sizeof(vertexAttr) / sizeof(vertexAttr[0]));
	format.CalcOffsetsAndSizeInterleaved();

	VertexData vertexData;
	vertexData.vertexFormat = format;
	vertexData.primitiveMode = RenderPrimitiveMode::TriangleStrip;
	vertexData.vertexData = vertexBuffer;
	vertexData.vertexDataSize = sizeof(vertexBuffer);
	vertexData.vertexCount = sizeof(vertexBuffer) / sizeof(vertexBuffer[0]);

	quadMeshId = meshManager->CreateMesh();
	meshManager->Upload(quadMeshId, vertexData);

	initUpdateShaderId = shaderManager->FindShaderByPath(kokko::ConstStringView("engine/shaders/particles/particle_init_update.glsl"));
	emitShaderId = shaderManager->FindShaderByPath(kokko::ConstStringView("engine/shaders/particles/particle_emit.glsl"));
	simulateShaderId = shaderManager->FindShaderByPath(kokko::ConstStringView("engine/shaders/particles/particle_simulate.glsl"));
	finishUpdateShaderId = shaderManager->FindShaderByPath(kokko::ConstStringView("engine/shaders/particles/particle_finish_update.glsl"));
	renderShaderId = shaderManager->FindShaderByPath(kokko::ConstStringView("engine/shaders/particles/particle_render.glsl"));

	// Create noise texture

	unsigned int valueCount = NoiseTextureSize * NoiseTextureSize * 4;
	Array<float> imageData(allocator);
	imageData.Resize(valueCount);

	for (unsigned int i = 0; i < valueCount; ++i)
		imageData[i] = Random::Float01();

	renderDevice->CreateTextures(1, &noiseTextureId);
	renderDevice->BindTexture(RenderTextureTarget::Texture2d, noiseTextureId);

	RenderCommandData::SetTextureStorage2D noiseTextureStorage{
		RenderTextureTarget::Texture2d, 1, RenderTextureSizedFormat::RGBA32F, NoiseTextureSize, NoiseTextureSize
	};
	renderDevice->SetTextureStorage2D(&noiseTextureStorage);

	RenderCommandData::SetTextureSubImage2D noiseTextureImage{
		RenderTextureTarget::Texture2d, 0, 0, 0, NoiseTextureSize, NoiseTextureSize,
		RenderTextureBaseFormat::RGBA, RenderTextureDataType::Float, imageData.GetData()
	};
	renderDevice->SetTextureSubImage2D(&noiseTextureImage);

}

void ParticleSystem::Deinitialize()
{
	if (noiseTextureId != 0)
	{
		renderDevice->DestroyTextures(1, &noiseTextureId);
		noiseTextureId = 0;
	}
}

void ParticleSystem::Submit(const SubmitParameters& parameters)
{
	assert(data.count <= UINT16_MAX);

	for (unsigned int i = 1; i < data.count; ++i)
	{
		EmitterData& emitter = data.emitter[i];
		float depth = 0.0f; // TODO: Calculate
		parameters.commandList.AddToFullscreenViewport(RenderPass::Transparent, depth, static_cast<uint16_t>(i));
	}
}

void ParticleSystem::Render(const RenderParameters& parameters)
{
	KOKKO_PROFILE_FUNCTION();

	double currentTime = Time::GetRunningTime();
	float deltaTime = Time::GetDeltaTime();

	EmitterData& emitter = data.emitter[parameters.featureObjectId];
	const Mat4x4f& transform = data.transform[parameters.featureObjectId];

	emitter.emitAccumulation += emitter.emitRate * deltaTime;
	int emitCount = static_cast<int>(emitter.emitAccumulation);
	emitter.emitAccumulation -= emitCount;

	int noiseSeed = Random::Int(0, NoiseTextureSize * NoiseTextureSize - 1);

	UpdateParticleBlock updateUniforms;
	updateUniforms.emitPosition = Vec3f(0.0f, 1.5f, 0.0f);
	updateUniforms.emitPositionVariance = 0.05f;
	updateUniforms.initialVelocity = Vec3f(5.0f, 3.0f, 0.0f);
	updateUniforms.initialVelocityVariance = 1.2f;
	updateUniforms.gravity = Vec3f(0.0f, -9.82f, 0.0f);
	updateUniforms.noiseTextureSize = NoiseTextureSize;
	updateUniforms.noiseSeed = noiseSeed;
	updateUniforms.emitCount = emitCount;
	updateUniforms.particleLifetime = 5.0f;
	updateUniforms.particleSize = 0.04f;
	updateUniforms.bounceEnergy = 0.9f;
	updateUniforms.currentTime = static_cast<float>(currentTime);
	updateUniforms.deltaTime = deltaTime;

	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, emitter.bufferIds[Buffer_UpdateUniforms]);
	renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(UpdateParticleBlock), &updateUniforms);

	TransformUniformBlock transformUniforms;
	transformUniforms.M = transform;
	transformUniforms.MV = parameters.fullscreenViewport.view.inverse * transform;
	transformUniforms.MVP = parameters.fullscreenViewport.viewProjection * transform;

	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, emitter.bufferIds[Buffer_RenderTransform]);
	renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(TransformUniformBlock), &transformUniforms);

	RenderCommandData::MemoryBarrier shaderStorageBarrier{};
	shaderStorageBarrier.shaderStorage = true;

	RenderCommandData::MemoryBarrier shaderStorageAndDrawIndirectBarrier{};
	shaderStorageAndDrawIndirectBarrier.shaderStorage = true;
	shaderStorageAndDrawIndirectBarrier.command = true;

	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 0, emitter.bufferIds[Buffer_Position]);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 1, emitter.bufferIds[Buffer_Velocity]);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 2, emitter.bufferIds[Buffer_Lifetime]);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 3, emitter.bufferIds[Buffer_DeadList]);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 4, emitter.aliveListCurrent);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 5, emitter.aliveListNext);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 6, emitter.bufferIds[Buffer_Counter]);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 7, emitter.bufferIds[Buffer_Indirect]);
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, emitter.bufferIds[Buffer_UpdateUniforms]);

	// Init update step

	const ShaderData& initUpdateShader = shaderManager->GetShaderData(initUpdateShaderId);
	renderDevice->UseShaderProgram(initUpdateShader.driverId);
	renderDevice->DispatchCompute(1, 1, 1);

	// Emit step

	// Make sure the indirect buffer updates are visible to the dispatch command
	renderDevice->MemoryBarrier(shaderStorageBarrier);

	renderDevice->BindBuffer(RenderBufferTarget::DispatchIndirectBuffer, emitter.bufferIds[Buffer_Indirect]);

	const ShaderData& emitShader = shaderManager->GetShaderData(emitShaderId);
	renderDevice->UseShaderProgram(emitShader.driverId);

	const kokko::TextureUniform* tu = emitShader.uniforms.FindTextureUniformByNameHash("noise_texture"_hash);
	if (tu != nullptr)
	{
		renderDevice->SetUniformInt(tu->uniformLocation, 0);
		renderDevice->SetActiveTextureUnit(0);
		renderDevice->BindTexture(RenderTextureTarget::Texture2d, noiseTextureId);
	}

	renderDevice->DispatchComputeIndirect(IndirectOffsetEmit);

	// Simulate step

	// Shader storage writes need to be made visible to the next step
	renderDevice->MemoryBarrier(shaderStorageBarrier);

	const ShaderData& simulateShader = shaderManager->GetShaderData(simulateShaderId);
	renderDevice->UseShaderProgram(simulateShader.driverId);
	renderDevice->DispatchComputeIndirect(IndirectOffsetSimulate);

	// Finish update step

	// Shader storage writes need to be made visible to the next step
	renderDevice->MemoryBarrier(shaderStorageBarrier);

	const ShaderData& finishUpdateShader = shaderManager->GetShaderData(finishUpdateShaderId);
	renderDevice->UseShaderProgram(finishUpdateShader.driverId);
	renderDevice->DispatchCompute(1, 1, 1);

	// Render step

	// Make sure changes to indirect arguments and particle buffers are visible
	renderDevice->MemoryBarrier(shaderStorageAndDrawIndirectBarrier);

	renderDevice->BindBuffer(RenderBufferTarget::DrawIndirectBuffer, emitter.bufferIds[Buffer_Indirect]);
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Viewport, parameters.fullscreenViewport.uniformBlockObject);
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, emitter.bufferIds[Buffer_RenderTransform]);

	const ShaderData& renderShader = shaderManager->GetShaderData(renderShaderId);
	renderDevice->UseShaderProgram(renderShader.driverId);

	const MeshDrawData* draw = meshManager->GetDrawData(quadMeshId);
	renderDevice->BindVertexArray(draw->vertexArrayObject);
	renderDevice->DrawIndirect(draw->primitiveMode, IndirectOffsetRender);

	// Swap alive lists

	unsigned int tempList = emitter.aliveListNext;
	emitter.aliveListNext = emitter.aliveListCurrent;
	emitter.aliveListCurrent = tempList;
}

void ParticleSystem::NotifyUpdatedTransforms(size_t count, const Entity* entities, const Mat4x4f* transforms)
{
	for (size_t i = 0; i < count; ++i)
	{
		Entity entity = entities[i];
		ParticleEmitterId id = Lookup(entity);
		if (id != ParticleEmitterId::Null)
			data.transform[id.i] = transforms[i];
	}
}

ParticleEmitterId ParticleSystem::Lookup(Entity e)
{
	auto* pair = entityMap.Lookup(e.id);
	return pair != nullptr ? pair->second : ParticleEmitterId::Null;
}

ParticleEmitterId ParticleSystem::AddEmitter(Entity entity)
{
	ParticleEmitterId id;
	this->AddEmitters(1, &entity, &id);
	return id;
}

void ParticleSystem::AddEmitters(unsigned int count, const Entity* entities, ParticleEmitterId* emitterIdsOut)
{
	if (data.count + count > data.allocated)
		this->ReallocateEmitters(data.count + count);

	for (unsigned int i = 0; i < count; ++i)
	{
		unsigned int id = data.count + i;

		Entity e = entities[i];

		auto mapPair = entityMap.Insert(e.id);
		mapPair->second.i = id;

		data.entity[id] = e;
		data.transform[id] = Mat4x4f();
		data.emitter[id] = EmitterData{};

		InitializeEmitter(ParticleEmitterId{ id });

		emitterIdsOut[i].i = id;
	}

	data.count += count;
}

void ParticleSystem::RemoveEmitter(ParticleEmitterId id)
{
	assert(id != ParticleEmitterId::Null);
	assert(id.i < data.count);

	// Remove from entity map
	Entity entity = data.entity[id.i];
	auto* pair = entityMap.Lookup(entity.id);
	if (pair != nullptr)
		entityMap.Remove(pair);

	DeinitializeEmitter(id);

	if (data.count > 2 && id.i + 1 < data.count) // We need to swap another object
	{
		unsigned int swapIdx = data.count - 1;

		// Update the swapped objects id in the entity map
		auto* swapKv = entityMap.Lookup(data.entity[swapIdx].id);
		if (swapKv != nullptr)
			swapKv->second = id;

		data.entity[id.i] = data.entity[swapIdx];
		data.transform[id.i] = data.transform[swapIdx];
		data.emitter[id.i] = data.emitter[swapIdx];
	}

	--data.count;
}

void ParticleSystem::RemoveAll()
{
	for (unsigned int i = 1; i < data.count; ++i)
	{
		DeinitializeEmitter(ParticleEmitterId{ i });
	}

	entityMap.Clear();
	data.count = 1;
}

float ParticleSystem::GetEmitRate(ParticleEmitterId id) const
{
	assert(id != ParticleEmitterId::Null);
	assert(id.i < data.count);
	return data.emitter[id.i].emitRate;
}

void ParticleSystem::SetEmitRate(ParticleEmitterId id, float rate)
{
	assert(id != ParticleEmitterId::Null);
	assert(id.i < data.count);
	data.emitter[id.i].emitRate = rate;
}

void ParticleSystem::ReallocateEmitters(unsigned int requiredCount)
{
	if (requiredCount <= data.allocated)
		return;

	static const unsigned int MinAllocation = 16;
	if (requiredCount < MinAllocation)
		requiredCount = MinAllocation;

	requiredCount = static_cast<unsigned int>(Math::UpperPowerOfTwo(requiredCount));

	// Reserve same amount in entity map
	entityMap.Reserve(requiredCount);

	InstanceData newData;
	unsigned int bytes = requiredCount * (sizeof(Entity) + sizeof(Mat4x4f) + sizeof(EmitterData));

	newData.buffer = this->allocator->Allocate(bytes, "ParticleSystem InstanceData buffer");
	newData.count = data.count;
	newData.allocated = requiredCount;

	newData.entity = static_cast<Entity*>(newData.buffer);
	newData.transform = reinterpret_cast<Mat4x4f*>(newData.entity + requiredCount);
	newData.emitter = reinterpret_cast<EmitterData*>(newData.transform + requiredCount);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.entity, data.entity, data.count * sizeof(Entity));
		std::memcpy(newData.transform, data.transform, data.count * sizeof(Mat4x4f));
		std::memcpy(newData.emitter, data.emitter, data.count * sizeof(EmitterData));

		this->allocator->Deallocate(data.buffer);
	}
	else
		newData.count = 1;

	data = newData;
}

void ParticleSystem::InitializeEmitter(ParticleEmitterId id)
{
	// Create the GPU buffers we need for updating and rendering our particles

	EmitterData& emitter = data.emitter[id.i];

	renderDevice->CreateBuffers(Buffer_COUNT, emitter.bufferIds);

	size_t IntSize = sizeof(int);
	size_t Vec2Size = sizeof(float) * 2;
	size_t Vec4Size = sizeof(float) * 4;

	size_t posBufferSize = Vec4Size * MaxParticleCount;
	size_t velBufferSize = Vec4Size * MaxParticleCount;
	size_t lifeBufferSize = Vec2Size * MaxParticleCount;
	size_t indexBufferSize = IntSize * MaxParticleCount;
	size_t indirectBufferSize = sizeof(unsigned int) * 4 * 3;
	size_t computeUniformBufferSize = sizeof(UpdateParticleBlock);
	size_t renderUniformBufferSize = sizeof(TransformUniformBlock);

	RenderCommandData::SetBufferStorage bufferStorage{};
	bufferStorage.target = RenderBufferTarget::ShaderStorageBuffer;

	RenderCommandData::SetBufferStorage uniformStorage{};
	uniformStorage.target = RenderBufferTarget::UniformBuffer;
	uniformStorage.dynamicStorage = true;

	// Position buffer
	renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, emitter.bufferIds[Buffer_Position]);

	bufferStorage.size = posBufferSize;
	renderDevice->SetBufferStorage(&bufferStorage);

	// Velocity buffer
	renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, emitter.bufferIds[Buffer_Velocity]);

	bufferStorage.size = velBufferSize;
	renderDevice->SetBufferStorage(&bufferStorage);

	// Lifetime buffer
	renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, emitter.bufferIds[Buffer_Lifetime]);

	bufferStorage.size = lifeBufferSize;
	renderDevice->SetBufferStorage(&bufferStorage);

	// Dead list buffer
	{
		renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, emitter.bufferIds[Buffer_DeadList]);

		RenderCommandData::SetBufferStorage deadListStorage{};
		deadListStorage.target = RenderBufferTarget::ShaderStorageBuffer;
		deadListStorage.size = indexBufferSize;
		deadListStorage.mapWriteAccess = true;
		renderDevice->SetBufferStorage(&deadListStorage);

		RenderCommandData::MapBufferRange map{};
		map.target = RenderBufferTarget::ShaderStorageBuffer;
		map.offset = 0;
		map.length = indexBufferSize;
		map.writeAccess = true;
		map.invalidateBuffer = true;

		unsigned int* buffer = static_cast<unsigned int*>(renderDevice->MapBufferRange(&map));

		for (unsigned int i = 0; i < MaxParticleCount; ++i)
			buffer[i] = i;

		renderDevice->UnmapBuffer(RenderBufferTarget::ShaderStorageBuffer);
	}

	// Alive list 0 buffer
	renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, emitter.bufferIds[Buffer_AliveList0]);

	bufferStorage.size = indexBufferSize;
	renderDevice->SetBufferStorage(&bufferStorage);

	// Alive list 1 buffer
	renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, emitter.bufferIds[Buffer_AliveList1]);

	bufferStorage.size = indexBufferSize;
	renderDevice->SetBufferStorage(&bufferStorage);

	// Counter buffer
	{
		renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, emitter.bufferIds[Buffer_Counter]);

		unsigned int counters[] = {
			0, MaxParticleCount, 0, 0
		};

		RenderCommandData::SetBufferStorage counterStorage{};
		counterStorage.target = RenderBufferTarget::ShaderStorageBuffer;
		counterStorage.size = sizeof(counters);
		counterStorage.data = counters;
		renderDevice->SetBufferStorage(&counterStorage);
	}

	// Indirect buffer
	renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, emitter.bufferIds[Buffer_Indirect]);

	bufferStorage.size = indirectBufferSize;
	renderDevice->SetBufferStorage(&bufferStorage);

	// Update uniform buffer
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, emitter.bufferIds[Buffer_UpdateUniforms]);

	uniformStorage.size = computeUniformBufferSize;
	renderDevice->SetBufferStorage(&uniformStorage);

	// Render uniform buffer
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, emitter.bufferIds[Buffer_RenderTransform]);

	uniformStorage.size = renderUniformBufferSize;
	renderDevice->SetBufferStorage(&uniformStorage);

	// Set current and next alive lists
	emitter.aliveListCurrent = emitter.bufferIds[Buffer_AliveList0];
	emitter.aliveListNext = emitter.bufferIds[Buffer_AliveList1];
}

void ParticleSystem::DeinitializeEmitter(ParticleEmitterId id)
{
	EmitterData& emitter = data.emitter[id.i];

	if (emitter.bufferIds[0] != 0)
	{
		renderDevice->DestroyBuffers(Buffer_COUNT, emitter.bufferIds);

		for (unsigned int i = 0; i < Buffer_COUNT; ++i)
			emitter.bufferIds[i] = 0;
	}
}

}
