#pragma once

#include <cstddef>

#include "Core/HashMap.hpp"

#include "Graphics/TerrainQuadTree.hpp"

#include "Rendering/CustomRenderer.hpp"

#include "Resources/MaterialData.hpp"
#include "Resources/MeshData.hpp"

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

struct TerrainParameters
{
	int terrainResolution = 128;
	float terrainSize = 64.0f;
	Vec2f textureScale = Vec2f(0.25f, 0.25f);
	float minHeight = -0.10f;
	float maxHeight = 0.10f;
};

class TerrainSystem : public CustomRenderer
{
public:
	TerrainSystem(Allocator* allocator, RenderDevice* renderDevice,
		MeshManager* meshManager, MaterialManager* materialManager, ShaderManager* shaderManager);
	~TerrainSystem();

	void Initialize();

	TerrainId Lookup(Entity e);

	TerrainId AddTerrain(Entity entity, const TerrainParameters& params);
	void RemoveTerrain(TerrainId id);

	void RemoveAll();

	int GetResolution(TerrainId id) const { return data.param[id.i].terrainResolution; }

	float GetSize(TerrainId id) const { return data.param[id.i].terrainSize; }
	void SetSize(TerrainId id, float size) { data.param[id.i].terrainSize = size; }

	float GetMinHeight(TerrainId id) const { return data.param[id.i].minHeight; }
	void SetMinHeight(TerrainId id, float minHeight) { data.param[id.i].minHeight = minHeight; }

	float GetMaxHeight(TerrainId id) const { return data.param[id.i].maxHeight; }
	void SetMaxHeight(TerrainId id, float maxHeight) { data.param[id.i].maxHeight = maxHeight; }

	Vec2f GetTextureScale(TerrainId id) const { return data.param[id.i].textureScale; }
	void SetTextureScale(TerrainId id, Vec2f scale) { data.param[id.i].textureScale = scale; }

	void RegisterCustomRenderer(Renderer* renderer);

	virtual void AddRenderCommands(const CustomRenderer::CommandParams& params) override final;
	virtual void RenderCustom(const CustomRenderer::RenderParams& params) override final;

private:
	Allocator* allocator;
	RenderDevice* renderDevice;
	MeshManager* meshManager;
	MaterialManager* materialManager;
	ShaderManager* shaderManager;

	kokko::TerrainQuadTree quadTree;

	MaterialId terrainMaterial;

	HashMap<unsigned int, TerrainId> entityMap;

	struct ResourceData
	{
		unsigned int vertexArrayId;
		unsigned int uniformBufferId;
		unsigned int textureId;
		MeshId meshId;
		uint16_t* heightData;
	};

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void* buffer;

		Entity* entity;
		TerrainParameters* param;
		ResourceData* resource;
	}
	data;

	void InitializeTerrain(TerrainId id);
	void DeinitializeTerrain(TerrainId id);

	void Reallocate(size_t required);

	void RenderTerrain(TerrainId id, const MaterialData& material, const RenderViewport& viewport);
};
