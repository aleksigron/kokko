#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/HashMap.hpp"

#include "Rendering/CustomRenderer.hpp"
#include "Graphics/TransformUpdateReceiver.hpp"

#include "Resources/MeshData.hpp"
#include "Resources/ShaderId.hpp"

class Allocator;
class RenderDevice;
class ShaderManager;
class MeshManager;
class Renderer;

struct Entity;

struct ParticleEmitterId
{
	unsigned int i;

	bool operator==(const ParticleEmitterId& other) { return i == other.i; }
	bool operator!=(const ParticleEmitterId& other) { return operator==(other) == false; }

	static const ParticleEmitterId Null;
};

class ParticleSystem : public CustomRenderer, public TransformUpdateReceiver
{
public:
	ParticleSystem(Allocator* allocator, RenderDevice* renderDevice,
		ShaderManager* shaderManager, MeshManager* meshManager);
	ParticleSystem(const ParticleSystem&) = delete;
	ParticleSystem(ParticleSystem&&) = delete;
	~ParticleSystem();

	ParticleSystem& operator=(const ParticleSystem&) = delete;
	ParticleSystem& operator=(ParticleSystem&&) = delete;

	void Initialize();
	void Deinitialize();

	void RegisterCustomRenderer(Renderer* renderer);

	virtual void AddRenderCommands(const CommandParams& params) override final;
	virtual void RenderCustom(const RenderParams& params) override final;

	virtual void NotifyUpdatedTransforms(
		size_t count, const Entity* entities, const Mat4x4f* transforms) override final;

	ParticleEmitterId Lookup(Entity e);

	ParticleEmitterId AddEmitter(Entity entity);
	void AddEmitters(unsigned int count, const Entity* entities, ParticleEmitterId* emitterIdsOut);
	void RemoveEmitter(ParticleEmitterId id);
	void RemoveAll();

	float GetEmitRate(ParticleEmitterId id) const;
	void SetEmitRate(ParticleEmitterId id, float rate);

private:
	static const size_t MaxParticleCount = 16 * 1024;
	static const unsigned int NoiseTextureSize = 64;

	static const intptr_t IndirectOffsetEmit = 0;
	static const intptr_t IndirectOffsetSimulate = 16;
	static const intptr_t IndirectOffsetRender = 32;

	Allocator* allocator;
	RenderDevice* renderDevice;
	ShaderManager* shaderManager;
	MeshManager* meshManager;

	MeshId quadMeshId;
	ShaderId initUpdateShaderId;
	ShaderId emitShaderId;
	ShaderId simulateShaderId;
	ShaderId finishUpdateShaderId;
	ShaderId renderShaderId;

	unsigned int noiseTextureId;

	enum Buffer {
		Buffer_Position,
		Buffer_Velocity,
		Buffer_Lifetime,
		Buffer_DeadList,
		Buffer_AliveList0,
		Buffer_AliveList1,
		Buffer_Counter,
		Buffer_Indirect,
		Buffer_UpdateUniforms,
		Buffer_RenderTransform,

		Buffer_COUNT
	};

	struct EmitterData
	{
		unsigned int bufferIds[Buffer_COUNT];

		unsigned int aliveListCurrent;
		unsigned int aliveListNext;

		float emitAccumulation;
		float emitRate;

		EmitterData() :
			aliveListCurrent(0),
			aliveListNext(0),
			emitAccumulation(0),
			emitRate(0)
		{
			for (size_t i = 0; i < Buffer_COUNT; ++i)
				bufferIds[i] = 0;
		}
	};

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void* buffer;

		Entity* entity;
		Mat4x4f* transform;
		EmitterData* emitter;
	}
	data;

	HashMap<unsigned int, ParticleEmitterId> entityMap;

	void ReallocateEmitters(unsigned int requiredCount);

	void InitializeEmitter(ParticleEmitterId id);
	void DeinitializeEmitter(ParticleEmitterId id);
};
