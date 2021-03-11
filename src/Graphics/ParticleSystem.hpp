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
	static const size_t MaxParticleCount = 4 * 1024;
	static const size_t ParticlesPerWorkgroup = 256;

	RenderDevice* renderDevice;
	ShaderManager* shaderManager;
	MeshManager* meshManager;

	MeshId quadMeshId;
	ShaderId updateShaderId;
	ShaderId renderShaderId;

	unsigned int customRenderCallback;

	unsigned int particleCount;

	static const unsigned int BufferCount = 4;
	enum Buffer {
		Buffer_Position,
		Buffer_Velocity,
		Buffer_UpdateUniforms,
		Buffer_RenderTransform
	};
	unsigned int bufferIds[BufferCount];

public:
	ParticleSystem(RenderDevice* renderDevice, ShaderManager* shaderManager, MeshManager* meshManager);
	ParticleSystem(const ParticleSystem&) = delete;
	ParticleSystem(ParticleSystem&&) = delete;
	~ParticleSystem();

	ParticleSystem& operator=(const ParticleSystem&) = delete;
	ParticleSystem& operator=(ParticleSystem&&) = delete;

	void Initialize(Renderer* renderer);

	virtual void AddRenderCommands(const CommandParams& params) override final;
	virtual void RenderCustom(const RenderParams& params) override final;
};
