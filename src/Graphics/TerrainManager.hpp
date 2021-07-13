#pragma once

#include "Engine/ComponentSystemDefaultImpl.hpp"

#include "Graphics/TerrainInstance.hpp"

#include "Rendering/CustomRenderer.hpp"

#include "Resources/MaterialData.hpp"

class Allocator;
class RenderDevice;
class MeshManager;
class MaterialManager;
class ShaderManager;
class Renderer;

struct MaterialData;

using TerrainId = ComponentSystemDefaultImpl<TerrainInstance>::ComponentId;

class TerrainManager : public CustomRenderer, public ComponentSystemDefaultImpl<TerrainInstance>
{
public:
	TerrainManager(Allocator* allocator, RenderDevice* renderDevice,
		MeshManager* meshManager, MaterialManager* materialManager, ShaderManager* shaderManager);
	~TerrainManager();

	void Initialize();

	void RegisterCustomRenderer(Renderer* renderer);

	virtual void AddRenderCommands(const CustomRenderer::CommandParams& params) override final;
	virtual void RenderCustom(const CustomRenderer::RenderParams& params) override final;

	void InitializeTerrain(TerrainId id);
	void DeinitializeTerrain(TerrainId id);

private:
	Allocator* allocator;
	RenderDevice* renderDevice;
	MeshManager* meshManager;
	MaterialManager* materialManager;
	ShaderManager* shaderManager;

	MaterialId terrainMaterial;

	void RenderTerrain(TerrainInstance& terrain, const MaterialData& material, const RenderViewport& viewport);
};
