#pragma once

#include <cstddef>

#include "Core/HashMap.hpp"

#include "Graphics/TerrainInstance.hpp"

#include "Rendering/CustomRenderer.hpp"

#include "Resources/MaterialData.hpp"

class Allocator;
class RenderDevice;
class MeshManager;
class MaterialManager;
class ShaderManager;
class Renderer;

struct Entity;
struct MaterialData;

struct TerrainId
{
	unsigned int i;

	bool operator==(TerrainId other) { return i == other.i; }
	bool operator!=(TerrainId other) { return !operator==(other); }

	static const TerrainId Null;
};

class TerrainSystem : public CustomRenderer
{
public:
	TerrainSystem(Allocator* allocator, RenderDevice* renderDevice,
		MeshManager* meshManager, MaterialManager* materialManager, ShaderManager* shaderManager);
	~TerrainSystem();

	void Initialize();

	TerrainId Lookup(Entity e);

	TerrainId AddTerrain(Entity entity);
	void RemoveTerrain(TerrainId id);

	void RemoveAll();

	Entity GetEntity(TerrainId id) const;
	const TerrainInstance& GetTerrainData(TerrainId id) const;
	void SetTerrainData(TerrainId id, const TerrainInstance& instance);

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

	HashMap<unsigned int, TerrainId> entityMap;

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void* buffer;

		Entity* entity;
		TerrainInstance* data;
	}
	data;

	void Reallocate(size_t required);

	void RenderTerrain(TerrainInstance& terrain, const MaterialData& material, const RenderViewport& viewport);
};
