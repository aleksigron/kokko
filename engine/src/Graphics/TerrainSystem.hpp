#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/StringView.hpp"

#include "Graphics/TerrainQuadTree.hpp"

#include "Graphics/GraphicsFeature.hpp"

#include "Resources/TextureId.hpp"
#include "Resources/ShaderId.hpp"

class Allocator;
class RenderDevice;
class MeshManager;
class ShaderManager;
class TextureManager;
class Renderer;

struct Entity;

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

class TerrainSystem : public GraphicsFeature
{
public:
	TerrainSystem(
		Allocator* allocator,
		kokko::render::Device* renderDevice,
		ShaderManager* shaderManager,
		TextureManager* textureManager);
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

	TextureId GetAlbedoTextureId(TerrainId id) const;
	void SetAlbedoTexture(TerrainId id, TextureId textureId, render::TextureId textureObject);

	TextureId GetRoughnessTextureId(TerrainId id) const;
	void SetRoughnessTexture(TerrainId id, TextureId textureId, render::TextureId textureObject);

	virtual void Submit(const SubmitParameters& parameters) override;
	virtual void Render(const RenderParameters& parameters) override;

private:
	Allocator* allocator;
	kokko::render::Device* renderDevice;
	ShaderManager* shaderManager;
	TextureManager* textureManager;
	
	uint32_t uniformBlockStride;
	uint32_t uniformBlocksAllocated;
	render::BufferId uniformBufferId;

	ShaderId terrainShader;

	HashMap<unsigned int, TerrainId> entityMap;

	static constexpr size_t MeshTypeCount = 9;

	struct VertexData
	{
		render::VertexArrayId vertexArrays[MeshTypeCount];
		render::BufferId vertexBuffer;
		render::BufferId indexBuffers[MeshTypeCount];
		int indexCounts[MeshTypeCount];
	};
	VertexData vertexData;

	render::SamplerId textureSampler;

	struct TextureInfo
	{
		TextureId textureId;
		render::TextureId textureObjectId;
	};

	struct TerrainTextures
	{
		TextureInfo albedoTexture;
		TextureInfo roughnessTexture;
	};

	struct InstanceData
	{
		size_t count;
		size_t allocated;
		void* buffer;

		Entity* entity;
		Vec2f* textureScale;
		TerrainTextures* textures;
		TerrainQuadTree* quadTree;
	}
	data;

	Array<uint8_t> uniformStagingBuffer;

	void InitializeTerrain(TerrainId id, const TerrainParameters& params);
	void DeinitializeTerrain(TerrainId id);

	void Reallocate(size_t required);

	void CreateVertexAndIndexData();
};

}
