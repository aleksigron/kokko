#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/Optional.hpp"
#include "Core/StringView.hpp"
#include "Core/Uid.hpp"

#include "Graphics/TerrainQuadTree.hpp"

#include "Graphics/GraphicsFeature.hpp"

#include "Resources/TextureId.hpp"
#include "Resources/ShaderId.hpp"

class Allocator;

struct Entity;

namespace kokko
{

class AssetLoader;
class MeshManager;
class Renderer;
class ShaderManager;
class TextureManager;

namespace render
{
class Device;
}

struct TerrainId
{
	unsigned int i;

	bool operator==(TerrainId other) { return i == other.i; }
	bool operator!=(TerrainId other) { return !operator==(other); }

	static const TerrainId Null;
};

/*
struct TerrainParameters
{
	float terrainSize = 64.0f;
	Vec2f textureScale = Vec2f(0.25f, 0.25f);
	float heightOrigin = -1.0f;
	float heightRange = 2.0f;
};
*/

class TerrainSystem : public GraphicsFeature
{
public:
	TerrainSystem(
		Allocator* allocator,
		AssetLoader* assetLoader,
		render::Device* renderDevice,
		ShaderManager* shaderManager,
		TextureManager* textureManager);
	~TerrainSystem();

	void Initialize();

	TerrainId Lookup(Entity e);

	TerrainId AddTerrain(Entity entity);
	void RemoveTerrain(TerrainId id);

	void RemoveAll();

	float GetSize(TerrainId id) const { return instances[id.i].quadTree.GetSize(); }
	void SetSize(TerrainId id, float size) { instances[id.i].quadTree.SetSize(size); }

	float GetBottom(TerrainId id) const { return instances[id.i].quadTree.GetBottom(); }
	void SetBottom(TerrainId id, float bottom) { instances[id.i].quadTree.SetBottom(bottom); }

	float GetHeight(TerrainId id) const { return instances[id.i].quadTree.GetHeight(); }
	void SetHeight(TerrainId id, float height) { instances[id.i].quadTree.SetHeight(height); }

	Vec2f GetTextureScale(TerrainId id) const { return instances[id.i].textureScale; }
	void SetTextureScale(TerrainId id, Vec2f scale) { instances[id.i].textureScale = scale; }

	Optional<Uid> GetHeightTexture(TerrainId id) const;
	void SetHeightTexture(TerrainId id, Uid textureUid);

	TextureId GetAlbedoTextureId(TerrainId id) const;
	void SetAlbedoTexture(TerrainId id, TextureId textureId, render::TextureId textureObject);

	TextureId GetRoughnessTextureId(TerrainId id) const;
	void SetRoughnessTexture(TerrainId id, TextureId textureId, render::TextureId textureObject);

	float GetRoughnessValue(TerrainId id) const;
	void SetRoughnessValue(TerrainId id, float roughness);

	virtual void Submit(const SubmitParameters& parameters) override;
	virtual void Render(const RenderParameters& parameters) override;

private:
	Allocator* allocator;
	AssetLoader* assetLoader;
	render::Device* renderDevice;
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
		TextureId id = TextureId::Null;
		render::TextureId renderId = render::TextureId::Null;
	};

	struct TerrainInstance
	{
		Entity entity = Entity::Null;
		Vec2f textureScale = Vec2f(1.0f, 1.0f);

		bool hasHeightTextureUid = false;
		Uid heightTextureUid;

		uint16_t* heightmapPixels = nullptr;
		uint32_t heightmapSize = 0;

		TextureInfo albedoTexture;
		TextureInfo roughnessTexture;
		float roughnessValue = 1.0f;

		TerrainQuadTree quadTree;

		TerrainInstance() = default;
		explicit TerrainInstance(TerrainQuadTree&& quadTree);

		TerrainInstance& operator=(TerrainInstance&& other) noexcept;
	};

	Array<TerrainInstance> instances;
	Array<uint8_t> uniformStagingBuffer;

	bool LoadHeightmap(TerrainId id, Uid textureUid);
	void CreateVertexAndIndexData();
};

}
