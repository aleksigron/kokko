#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/HashMap.hpp"

#include "Graphics/GraphicsFeature.hpp"
#include "Graphics/TransformUpdateReceiver.hpp"

#include "Resources/MeshId.hpp"
#include "Resources/ShaderId.hpp"

class Allocator;
class RenderDevice;
class ShaderManager;
class ModelManager;
class Renderer;

struct Entity;

namespace kokko
{

struct ParticleEmitterId
{
	unsigned int i;

	bool operator==(const ParticleEmitterId& other) { return i == other.i; }
	bool operator!=(const ParticleEmitterId& other) { return operator==(other) == false; }

	static const ParticleEmitterId Null;
};

class ParticleSystem : public GraphicsFeature, public TransformUpdateReceiver
{
public:
	ParticleSystem(Allocator* allocator, kokko::render::Device* renderDevice,
		ShaderManager* shaderManager, ModelManager* modelManager);
	ParticleSystem(const ParticleSystem&) = delete;
	ParticleSystem(ParticleSystem&&) = delete;
	~ParticleSystem();

	ParticleSystem& operator=(const ParticleSystem&) = delete;
	ParticleSystem& operator=(ParticleSystem&&) = delete;

	void Initialize();
	void Deinitialize();

	virtual void Upload(const UploadParameters& parameters) override;

	virtual void Submit(const SubmitParameters& parameters) override;
	virtual void Render(const RenderParameters& parameters) override;

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
	kokko::render::Device* renderDevice;
	ShaderManager* shaderManager;
	ModelManager* modelManager;

	ModelId quadMeshId;
	ShaderId initUpdateShaderId;
	ShaderId emitShaderId;
	ShaderId simulateShaderId;
	ShaderId finishUpdateShaderId;
	ShaderId renderShaderId;

	render::TextureId noiseTextureId;

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
		render::BufferId bufferIds[Buffer_COUNT];
		render::BufferId aliveListCurrent;
		render::BufferId aliveListNext;

		float emitAccumulation;
		float emitRate;

		EmitterData() :
			aliveListCurrent(),
			aliveListNext(),
			emitAccumulation(0),
			emitRate(0)
		{
			for (size_t i = 0; i < Buffer_COUNT; ++i)
				bufferIds[i] = render::BufferId();
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

	void InitializeEmitter(kokko::render::Device* renderDevice, ParticleEmitterId id);
	void DeinitializeEmitter(kokko::render::Device* renderDevice, ParticleEmitterId id);
};

}
