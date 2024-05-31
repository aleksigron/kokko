#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/ArrayView.hpp"
#include "Core/Range.hpp"
#include "Core/SortedArray.hpp"
#include "Core/Uid.hpp"

struct cgltf_buffer_view;
struct cgltf_data;
struct cgltf_mesh;
struct cgltf_node;

namespace kokko
{

struct ModelData;
struct ModelMesh;
struct ModelCreateInfo;

class ModelLoader
{
public:
	ModelLoader(Allocator* allocator);

	bool LoadRuntime(ModelData* modelOut, Array<uint8_t>* geometryBufferOut, const ModelCreateInfo& createInfo);
	bool LoadGlbFromBuffer(ModelData* modelOut, Array<uint8_t>* geometryBufferOut, ArrayView<const uint8_t> buffer);

private:
	Allocator* allocator;

	ModelData* outputModel = nullptr;

	ArrayView<char> textBuffer;

	Array<uint8_t>* geometryBuffer = nullptr;
	size_t geometryBufferUsed = 0;

	SortedArray<cgltf_buffer_view*> uniqueGeometryBufferViews;
	Array<Range<size_t>> geometryBufferViewRangeMap;

	void Reset();
	
	void LoadGltfNode(int16_t parent, cgltf_data* data, cgltf_node* node);
	bool LoadGltfMesh(cgltf_mesh* cgltfMesh, ModelMesh& modelMeshOut);
};

} // namespace kokko
