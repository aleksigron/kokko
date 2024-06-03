#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/StringView.hpp"
#include "Core/Uid.hpp"

#include "Math/AABB.hpp"
#include "Math/Mat4x4.hpp"

#include "Rendering/RenderTypes.hpp"
#include "Rendering/RenderResourceId.hpp"
#include "Rendering/VertexFormat.hpp"

#include "Resources/ModelLoader.hpp"

class Allocator;

namespace kokko
{

namespace render
{
class Device;
}

struct MeshId;
struct ModelId;
class AssetLoader;
class MeshManager;

struct ModelNode
{
	int16_t parent;
	int16_t firstChild;
	int16_t nextSibling;
	int16_t meshIndex;
	Mat4x4f transform;
};

struct ModelMesh
{
	uint16_t partOffset;
	uint16_t partCount;

	RenderIndexType indexType;
	RenderPrimitiveMode primitiveMode;

	const char* name;

	AABB aabb;
};

struct ModelMeshPart
{
	uint32_t uniqueVertexCount;
	uint32_t indexOffset;
	uint32_t count;

	VertexFormat vertexFormat;
	render::VertexArrayId vertexArrayId;
};

struct ModelData
{
	void* buffer = nullptr;

	kokko::Uid uid;

	ModelNode* nodes = nullptr;
	ModelMesh* meshes = nullptr;
	ModelMeshPart* meshParts = nullptr;
	VertexAttribute* attributes = nullptr;

	uint32_t nodeCount = 0;
	uint32_t meshCount = 0;
	uint32_t meshPartCount = 0;
	uint32_t attributeCount = 0;

	render::BufferId bufferId;

	bool hasUid = false;
};

struct ModelCreateInfo
{
	// For now, only support creating single mesh models at runtime

	VertexFormat vertexFormat;

	RenderPrimitiveMode primitiveMode = RenderPrimitiveMode::Triangles;

	const void* vertexData = nullptr;
	size_t vertexDataSize = 0;
	uint32_t vertexCount = 0;

	const void* indexData = nullptr;
	size_t indexDataSize = 0;
	uint32_t indexCount = 0;
	RenderIndexType indexType = RenderIndexType::None;
};

class ModelManager
{
public:
	ModelManager(Allocator* allocator, AssetLoader* assetLoader, render::Device* renderDevice);
	~ModelManager();
	
	// Model create & delete

	ModelId FindModelByUid(const kokko::Uid& uid);
	ModelId FindModelByPath(const ConstStringView& path);

	ModelId CreateModel(const ModelCreateInfo& modelCreateInfo);

	void RemoveModel(ModelId id);

	// Model info setters

	void SetMeshAABB(MeshId id, const AABB& bounds);

	// Model info getters

	Optional<Uid> GetModelUid(ModelId id) const;

	ArrayView<const ModelNode> GetModelNodes(ModelId id) const;
	ArrayView<const ModelMesh> GetModelMeshes(ModelId id) const;
	ArrayView<const ModelMeshPart> GetModelMeshParts(ModelId id) const;

private:
	Allocator* allocator;
	AssetLoader* assetLoader;
	render::Device* renderDevice;

	ModelLoader modelLoader;
	HashMap<Uid, uint32_t> uidMap;

	struct InstanceData
	{
		uint32_t slotsUsed = 0; // Slots at the start of the buffer that have been used at some point
		uint32_t allocated = 0; // Slots allocated in the buffer
		uint32_t freelistFirst = 0;

		void* buffer = nullptr;

		uint32_t* freelist = nullptr; // Each node points to the next node in the freelist
		ModelData* model = nullptr;
	}
	data;

	uint32_t AcquireSlot();
	void ReleaseSlot(uint32_t id);
	void Reallocate(uint32_t required);

	void CreateRenderData(ModelData& model, Array<uint8_t>& geometryBuffer);
	void ReleaseRenderData(ModelData& model);
};

} // namespace kokko
