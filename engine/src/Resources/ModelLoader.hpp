#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/ArrayView.hpp"
#include "Core/Range.hpp"
#include "Core/SortedArray.hpp"
#include "Core/Uid.hpp"

#include "Resources/ModelManager.hpp"

#include "Rendering/VertexFormat.hpp"

struct cgltf_buffer_view;
struct cgltf_data;
struct cgltf_mesh;
struct cgltf_node;

namespace kokko
{

struct MeshVertexData
{
	VertexFormat vertexFormat;

	const void* vertexData = nullptr;
	size_t vertexDataSize = 0;
	const void* indexData = nullptr;
	size_t indexDataSize = 0;
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;
	RenderIndexType indexType = RenderIndexType::None;
	RenderPrimitiveMode primitiveMode = RenderPrimitiveMode::Triangles;
};

class ModelLoader
{
public:
	ModelLoader(Allocator* allocator, ModelManager* modelManager);

	bool LoadFromBuffer(ModelData* modelOut, ArrayView<const uint8_t> buffer);

private:
	Allocator* allocator;
	ModelManager* modelManager;

	ModelData* outputModel = nullptr;

	ArrayView<char> textBuffer;

	void* geometryBuffer = nullptr;
	size_t geometryBufferBytes = 0;
	size_t geometryBufferUsed = 0;

	SortedArray<cgltf_buffer_view*> uniqueGeometryBufferViews;
	Array<Range<size_t>> geometryBufferViewRangeMap;
	
	void LoadNode(
		int16_t parent,
		cgltf_data* data,
		cgltf_node* node);

	bool LoadMesh(
		cgltf_mesh* cgltfMesh,
		ModelMesh& modelMeshOut);
};

} // namespace kokko
