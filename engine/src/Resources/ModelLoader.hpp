#pragma once

#include <cstdint>

#include "Core/ArrayView.hpp"
#include "Core/Uid.hpp"

#include "Resources/ModelManager.hpp"

struct cgltf_data;
struct cgltf_mesh;
struct cgltf_node;

struct MeshId;

namespace kokko
{

class ModelLoader
{
public:
	ModelLoader(ModelManager* modelManager);

	bool LoadFromBuffer(ModelManager::ModelData& model, ArrayView<const uint8_t> buffer);

private:
	Allocator* allocator;
	ModelManager* modelManager;

	ArrayView<char> textBuffer;
	Uid uid;
	
	void LoadNode(
		ModelManager::ModelData& model,
		int16_t parent,
		cgltf_data* data,
		cgltf_node* node);

	bool LoadMesh(
		cgltf_mesh* cgltfMesh,
		ModelMesh& modelMeshOut);
};

}
