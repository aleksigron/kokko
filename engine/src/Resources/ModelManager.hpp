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

class Allocator;

namespace kokko
{

namespace render
{
class Device;
}

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
	uint16_t primitiveOffset;
	uint16_t primitiveCount;

	RenderIndexType indexType = RenderIndexType::None;
	RenderPrimitiveMode primitiveMode = RenderPrimitiveMode::Triangles;

	const char* name;

	AABB aabb;
};

struct ModelPrimitive
{
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
	ModelPrimitive* primitives = nullptr;
	VertexAttribute* attributes = nullptr;

	uint32_t nodeCount = 0;
	uint32_t meshCount = 0;
	uint32_t primitiveCount = 0;
	uint32_t attributeCount = 0;

	render::BufferId bufferId;
};

class ModelManager
{
public:
	ModelManager(Allocator* allocator, AssetLoader* assetLoader, render::Device* renderDevice);
	~ModelManager();

	ModelId FindModelByUid(const kokko::Uid& uid);
	ModelId FindModelByPath(const ConstStringView& path);

	kokko::Uid GetModelUid(ModelId id) const;

	ArrayView<const ModelNode> GetModelNodes(ModelId id) const;
	ArrayView<const ModelMesh> GetModelMeshes(ModelId id) const;
	ArrayView<const ModelPrimitive> GetModelPrimitives(ModelId id) const;

private:
	Allocator* allocator;
	AssetLoader* assetLoader;
	render::Device* renderDevice;

	HashMap<kokko::Uid, uint32_t> uidMap;
	Array<ModelData> models;

	void CreateRenderData(ModelData& model, Array<uint8_t>& geometryBuffer);
};

} // namespace kokko
