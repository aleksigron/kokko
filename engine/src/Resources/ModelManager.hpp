#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/StringView.hpp"
#include "Core/Uid.hpp"

#include "Math/Mat4x4.hpp"

#include "Resources/MeshId.hpp"

class Allocator;

namespace kokko
{

class AssetLoader;
class MeshManager;

struct MeshUid;

struct ModelId
{
	unsigned int i;

	bool operator==(ModelId other) const { return other.i == i; }
	bool operator!=(ModelId other) const { return operator==(other) == false; }

	static const ModelId Null;
};

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
	MeshId meshId;
	const char* name;
};

struct ModelMeshId
{
	ModelId modelId;
	uint32_t meshIndex;
};

class ModelManager
{
public:
	ModelManager(Allocator* allocator, AssetLoader* assetLoader, MeshManager* meshManager);
	~ModelManager();

	ModelId FindModelByUid(const kokko::Uid& uid);
	ModelId FindModelByPath(const ConstStringView& path);

	kokko::Uid GetModelUid(ModelId id) const;

	ArrayView<const ModelMesh> GetModelMeshes(ModelId id) const;
	ArrayView<const ModelNode> GetModelNodes(ModelId id) const;

private:
	friend class ModelLoader;

	struct ModelData
	{
		void* buffer = nullptr;

		kokko::Uid uid;

		ModelNode* nodes = nullptr;
		uint32_t nodeCount = 0;

		uint32_t meshCount = 0;
		ModelMesh* meshes = nullptr;
	};

	Allocator* allocator;
	AssetLoader* assetLoader;
	MeshManager* meshManager;

	HashMap<kokko::Uid, uint32_t> uidMap;
	Array<ModelData> models;
};

} // namespace kokko
