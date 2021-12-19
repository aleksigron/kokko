#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"

#include "Graphics/TerrainQuadTree.hpp"
#include "Graphics/TerrainTileId.hpp"

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

namespace kokko
{

struct TerrainId
{
	unsigned int i;

	bool operator==(TerrainId other) { return i == other.i; }
	bool operator!=(TerrainId other) { return !operator==(other); }

	static const TerrainId Null;
};

struct TerrainParameters
{
	float terrainSize = 64.0f;
	Vec2f textureScale = Vec2f(0.25f, 0.25f);
	float heightOrigin = -1.0f;
	float heightRange = 2.0f;
};

class TerrainSystem : public CustomRenderer
{
public:
	TerrainSystem(Allocator* allocator, RenderDevice* renderDevice,
		MaterialManager * materialManager, ShaderManager* shaderManager);
	~TerrainSystem();

	void Initialize();

	TerrainId Lookup(Entity e);

	TerrainId AddTerrain(Entity entity, const TerrainParameters& params);
	void RemoveTerrain(TerrainId id);

	void RemoveAll();

	float GetSize(TerrainId id) const { return data.quadTree[id.i].GetSize(); }
	void SetSize(TerrainId id, float size) { data.quadTree[id.i].SetSize(size); }

	float GetBottom(TerrainId id) const { return data.quadTree[id.i].GetBottom(); }
	void SetBottom(TerrainId id, float bottom) { data.quadTree[id.i].SetBottom(bottom); }

	float GetHeight(TerrainId id) const { return data.quadTree[id.i].GetHeight(); }
	void SetHeight(TerrainId id, float height) { data.quadTree[id.i].SetHeight(height); }

	Vec2f GetTextureScale(TerrainId id) const { return data.textureScale[id.i]; }
	void SetTextureScale(TerrainId id, Vec2f scale) { data.textureScale[id.i] = scale; }

	void RegisterCustomRenderer(Renderer* renderer);

	virtual void AddRenderCommands(const CustomRenderer::CommandParams& params) override final;
	virtual void RenderCustom(const CustomRenderer::RenderParams& params) override final;

private:
	Allocator* allocator;
	RenderDevice* renderDevice;
	MaterialManager* materialManager;
	ShaderManager* shaderManager;
	
	unsigned int uniformBlockStride;
	unsigned int uniformBufferId;

	MaterialId terrainMaterial;

	HashMap<unsigned int, TerrainId> entityMap;

	struct VertexData
	{
		unsigned int vertexArray;
		unsigned int vertexBuffer;
		unsigned int indexBuffer;

		int indexCount;
	};
	VertexData vertexData;

	unsigned int textureSampler;

	struct InstanceData
	{
		size_t count;
		size_t allocated;
		void* buffer;

		Entity* entity;
		Vec2f* textureScale;
		TerrainQuadTree* quadTree;
	}
	data;

	Array<TerrainTileId> tilesToRender;

	Array<uint8_t> uniformStagingBuffer;

	void CreateVertexData();

	void InitializeTerrain(TerrainId id, const TerrainParameters& params);
	void DeinitializeTerrain(TerrainId id);

	void Reallocate(size_t required);

	void RenderTerrain(TerrainId id, const RenderViewport& viewport);
};

}
