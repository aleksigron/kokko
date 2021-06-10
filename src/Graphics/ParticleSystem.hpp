#pragma once

#include "Rendering/CustomRenderer.hpp"

#include "Resources/MeshData.hpp"
#include "Resources/ShaderId.hpp"

class Allocator;
class RenderDevice;
class ShaderManager;
class MeshManager;
class Renderer;

class ParticleSystem : public CustomRenderer
{
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

	static const unsigned int BufferCount = 10;
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
		Buffer_RenderTransform
	};
	unsigned int bufferIds[BufferCount];

	unsigned int aliveListCurrent;
	unsigned int aliveListNext;

	float emitAccumulation;
	float emitRate;

public:
	ParticleSystem(Allocator* allocator, RenderDevice* renderDevice,
		ShaderManager* shaderManager, MeshManager* meshManager);
	ParticleSystem(const ParticleSystem&) = delete;
	ParticleSystem(ParticleSystem&&) = delete;
	~ParticleSystem();

	ParticleSystem& operator=(const ParticleSystem&) = delete;
	ParticleSystem& operator=(ParticleSystem&&) = delete;

	void Initialize();

	void RegisterCustomRenderer(Renderer* renderer);

	virtual void AddRenderCommands(const CommandParams& params) override final;
	virtual void RenderCustom(const RenderParams& params) override final;
};
