#include "ParticleSystem.hpp"

#include <cstring>

#include "Math/Vec3.hpp"
#include "Math/Random.hpp"

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
	customRenderCallback(0),
	noiseTextureId(0)
{
	for (unsigned int i = 0; i < BufferCount; ++i)
		bufferIds[i] = 0;

	aliveListCurrent = 0;
	aliveListNext = 0;

	emitAccumulation = 0.0f;
	emitRate = 2000.0f;
}

ParticleSystem::~ParticleSystem()
{
	if (bufferIds[0] != 0)
	{
		renderDevice->DestroyBuffers(BufferCount, bufferIds);
	}

	if (noiseTextureId != 0)
	{
		renderDevice->DestroyTextures(1, &noiseTextureId);
	}
}

void ParticleSystem::Initialize(Renderer* renderer)
{
	customRenderCallback = renderer->AddCustomRenderer(this);

	float vertexBuffer[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};

	VertexAttribute vertexAttr[] = { VertexAttribute::pos3 };
	VertexData vertexData;
	vertexData.vertexFormat = VertexFormat(vertexAttr, sizeof(vertexAttr) / sizeof(vertexAttr[0]));
	vertexData.primitiveMode = RenderPrimitiveMode::TriangleStrip;
	vertexData.vertexData = vertexBuffer;
	vertexData.vertexCount = 4;

	quadMeshId = meshManager->CreateMesh();
	meshManager->Upload(quadMeshId, vertexData);

	initUpdateShaderId = shaderManager->GetIdByPath(StringRef("res/shaders/particles/particle_init_update.shader.json"));
	emitShaderId = shaderManager->GetIdByPath(StringRef("res/shaders/particles/particle_emit.shader.json"));
	simulateShaderId = shaderManager->GetIdByPath(StringRef("res/shaders/particles/particle_simulate.shader.json"));
	finishUpdateShaderId = shaderManager->GetIdByPath(StringRef("res/shaders/particles/particle_finish_update.shader.json"));
	renderShaderId = shaderManager->GetIdByPath(StringRef("res/shaders/particles/particle_render.shader.json"));

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

	// Create the GPU buffers we need for updating and rendering our particles

	renderDevice->CreateBuffers(BufferCount, bufferIds);

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
	renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, bufferIds[Buffer_Position]);

	bufferStorage.size = posBufferSize;
	renderDevice->SetBufferStorage(&bufferStorage);

	// Velocity buffer
	renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, bufferIds[Buffer_Velocity]);

	bufferStorage.size = velBufferSize;
	renderDevice->SetBufferStorage(&bufferStorage);

	// Lifetime buffer
	renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, bufferIds[Buffer_Lifetime]);

	bufferStorage.size = lifeBufferSize;
	renderDevice->SetBufferStorage(&bufferStorage);

	// Dead list buffer
	{
		renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, bufferIds[Buffer_DeadList]);

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
	renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, bufferIds[Buffer_AliveList0]);

	bufferStorage.size = indexBufferSize;
	renderDevice->SetBufferStorage(&bufferStorage);

	// Alive list 1 buffer
	renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, bufferIds[Buffer_AliveList1]);

	bufferStorage.size = indexBufferSize;
	renderDevice->SetBufferStorage(&bufferStorage);

	// Counter buffer
	{
		renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, bufferIds[Buffer_Counter]);

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
	renderDevice->BindBuffer(RenderBufferTarget::ShaderStorageBuffer, bufferIds[Buffer_Indirect]);

	bufferStorage.size = indirectBufferSize;
	renderDevice->SetBufferStorage(&bufferStorage);

	// Update uniform buffer
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, bufferIds[Buffer_UpdateUniforms]);

	uniformStorage.size = computeUniformBufferSize;
	renderDevice->SetBufferStorage(&uniformStorage);

	// Render uniform buffer
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, bufferIds[Buffer_RenderTransform]);

	uniformStorage.size = renderUniformBufferSize;
	renderDevice->SetBufferStorage(&uniformStorage);

	// Set current and next alive lists
	aliveListCurrent = bufferIds[Buffer_AliveList0];
	aliveListNext = bufferIds[Buffer_AliveList1];
}

void ParticleSystem::AddRenderCommands(const CommandParams& params)
{
	float depth = 0.0f; // TODO: Calculate
	params.commandList->AddDrawWithCallback(params.fullscreenViewport, RenderPass::Transparent, depth, params.callbackId);
}

void ParticleSystem::RenderCustom(const RenderParams& params)
{
	double currentTime = Time::GetRunningTime();
	float deltaTime = Time::GetDeltaTime();

	emitAccumulation += emitRate * deltaTime;
	int emitCount = static_cast<int>(emitAccumulation);
	emitAccumulation -= emitCount;

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

	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, bufferIds[Buffer_UpdateUniforms]);
	renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(UpdateParticleBlock), &updateUniforms);

	TransformUniformBlock transformUniforms;
	transformUniforms.M = Mat4x4f();
	transformUniforms.MV = params.viewport->view;
	transformUniforms.MVP = params.viewport->viewProjection;

	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, bufferIds[Buffer_RenderTransform]);
	renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(TransformUniformBlock), &transformUniforms);

	RenderCommandData::MemoryBarrier shaderStorageBarrier{};
	shaderStorageBarrier.shaderStorage = true;

	RenderCommandData::MemoryBarrier shaderStorageAndDrawIndirectBarrier{};
	shaderStorageAndDrawIndirectBarrier.shaderStorage = true;
	shaderStorageAndDrawIndirectBarrier.command = true;

	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 0, bufferIds[Buffer_Position]);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 1, bufferIds[Buffer_Velocity]);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 2, bufferIds[Buffer_Lifetime]);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 3, bufferIds[Buffer_DeadList]);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 4, aliveListCurrent);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 5, aliveListNext);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 6, bufferIds[Buffer_Counter]);
	renderDevice->BindBufferBase(RenderBufferTarget::ShaderStorageBuffer, 7, bufferIds[Buffer_Indirect]);
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, bufferIds[Buffer_UpdateUniforms]);

	// Init update step

	const ShaderData& initUpdateShader = shaderManager->GetShaderData(initUpdateShaderId);
	renderDevice->UseShaderProgram(initUpdateShader.driverId);
	renderDevice->DispatchCompute(1, 1, 1);

	// Emit step

	// Make sure the indirect buffer updates are visible to the dispatch command
	renderDevice->MemoryBarrier(shaderStorageBarrier);

	renderDevice->BindBuffer(RenderBufferTarget::DispatchIndirectBuffer, bufferIds[Buffer_Indirect]);

	const ShaderData& emitShader = shaderManager->GetShaderData(emitShaderId);
	renderDevice->UseShaderProgram(emitShader.driverId);

	const TextureUniform* tu = emitShader.uniforms.FindTextureUniformByNameHash("noise_texture"_hash);
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

	renderDevice->BindBuffer(RenderBufferTarget::DrawIndirectBuffer, bufferIds[Buffer_Indirect]);
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Viewport, params.viewport->uniformBlockObject);
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, bufferIds[Buffer_RenderTransform]);

	const ShaderData& renderShader = shaderManager->GetShaderData(renderShaderId);
	renderDevice->UseShaderProgram(renderShader.driverId);

	const MeshDrawData* draw = meshManager->GetDrawData(quadMeshId);
	renderDevice->BindVertexArray(draw->vertexArrayObject);
	renderDevice->DrawIndirect(draw->primitiveMode, IndirectOffsetRender);

	// Swap alive lists

	unsigned int tempList = aliveListNext;
	aliveListNext = aliveListCurrent;
	aliveListCurrent = tempList;
}
