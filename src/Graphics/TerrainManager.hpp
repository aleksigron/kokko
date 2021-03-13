#pragma once

#include "Rendering/CustomRenderer.hpp"

#include "Resources/MaterialData.hpp"

class Allocator;
class RenderDevice;
class MeshManager;
class MaterialManager;
class ShaderManager;
class Renderer;
class TerrainInstance;

class TerrainManager : public CustomRenderer
{
private:
	Allocator* allocator;
	RenderDevice* renderDevice;
	MeshManager* meshManager;
	MaterialManager* materialManager;
	TerrainInstance* terrainInstances;

	MaterialId terrainMaterial;

public:
	TerrainManager(Allocator* allocator, RenderDevice* renderDevice,
		MeshManager* meshManager, MaterialManager* materialManager);
	~TerrainManager();

	void Initialize(Renderer* renderer, ShaderManager* shaderManager);

	virtual void AddRenderCommands(const CustomRenderer::CommandParams& params) override final;
	virtual void RenderCustom(const CustomRenderer::RenderParams& params) override final;
};
