#include "Resources/ModelLoader.hpp"

#include <cassert>

#include "cgltf/cgltf.h"

#include "Core/CString.hpp"

#include "Memory/Allocator.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/ModelManager.hpp"

namespace
{

struct CountResult
{
	size_t nodeCount;
	size_t meshCount;
	size_t totalStringLength;

	CountResult() :
		nodeCount(0),
		meshCount(0),
		totalStringLength(0)
	{
	}

	explicit CountResult(cgltf_node* node) :
		nodeCount(1),
		meshCount(node->mesh ? 1 : 0),
		totalStringLength(node->mesh ? std::strlen(node->mesh->name) + 1 : 0)
	{
	}

	CountResult& operator+=(const CountResult& other)
	{
		nodeCount += other.nodeCount;
		meshCount += other.meshCount;
		totalStringLength += other.totalStringLength;
		return *this;
	}
};

CountResult CountNodesAndMeshes(cgltf_node* node)
{
	CountResult result(node);

	for (size_t i = 0; i < node->children_count; ++i)
		result += CountNodesAndMeshes(node->children[i]);

	return result;
}

}

namespace kokko
{

ModelLoader::ModelLoader(ModelManager* modelManager) :
	allocator(modelManager->allocator),
	modelManager(modelManager)
{
}

bool ModelLoader::LoadFromBuffer(ModelManager::ModelData& model, ArrayView<const uint8_t> buffer)
{
	cgltf_options options{};
	cgltf_data* data = nullptr;

	cgltf_result result = cgltf_parse(&options, buffer.GetData(), buffer.GetCount(), &data);
	if (result != cgltf_result_success)
	{
		KK_LOG_ERROR("ModelLoader: Failed to load model, couldn't parse glTF");
		return false;
	}
	
	// TODO: mesh path and callbacks
	cgltf_result loadResult = cgltf_load_buffers(&options, data, nullptr);
	if (loadResult != cgltf_result_success)
	{
		KK_LOG_ERROR("ModelLoader: Failed to load model, couldn't load glTF buffers");
		cgltf_free(data);
		return false;
	}

	if (data->scenes_count == 0)
	{
		KK_LOG_ERROR("ModelLoader: Failed to load model, glTF didn't contain any scenes");
		cgltf_free(data);
		return false;
	}

	cgltf_scene& scene = data->scenes[0];

	CountResult countResult;
	for (size_t i = 0; i < scene.nodes_count; ++i)
		countResult += CountNodesAndMeshes(scene.nodes[i]);

	if (countResult.nodeCount == 0 || countResult.meshCount == 0)
	{
		KK_LOG_ERROR("ModelLoader: Failed to load model, glTF didn't contain any meshes");
		cgltf_free(data);
		return false;
	}

	size_t bufferBytes = sizeof(ModelNode) * countResult.nodeCount +
		sizeof(ModelMesh) * countResult.meshCount + countResult.totalStringLength;

	model.buffer = allocator->Allocate(bufferBytes, "ModelLoader.LoadFromBuffer() model.buffer");

	textBuffer = ArrayView<char>(static_cast<char*>(model.buffer), countResult.totalStringLength);
	uid = model.uid;

	model.nodes = reinterpret_cast<ModelNode*>(textBuffer.GetData() + countResult.totalStringLength);
	model.meshes = reinterpret_cast<ModelMesh*>(model.nodes + countResult.nodeCount);

	// These are used as counters to know how many have been inserted
	model.nodeCount = 0; 
	model.meshCount = 0;

	uint32_t lastSiblingIndex = 0;
	for (size_t i = 0; i < scene.nodes_count; ++i)
	{
		if (i > 0)
			model.nodes[lastSiblingIndex].nextSibling = model.nodeCount;

		lastSiblingIndex = model.nodeCount;

		LoadNode(model, -1, data, scene.nodes[i]);
	}

	cgltf_free(data);

	textBuffer = ArrayView<char>();
	uid = Uid();

	return true;
}

void ModelLoader::LoadNode(
	ModelManager::ModelData& model,
	int16_t parent,
	cgltf_data* data,
	cgltf_node* node)
{
	int16_t thisNodeIndex = static_cast<int16_t>(model.nodeCount);
	ModelNode& modelNode = model.nodes[model.nodeCount];

	modelNode.meshIndex = -1;
	if (node->mesh != nullptr)
	{
		size_t meshIndex = model.meshCount;
		if (LoadMesh(node->mesh, model.meshes[meshIndex]))
		{
			modelNode.meshIndex = static_cast<int16_t>(meshIndex);
			model.meshCount += 1;
		}
	}

	if (node->has_matrix)
		std::memcpy(&(modelNode.transform), &(node->matrix), sizeof(float) * 16);
	else
		modelNode.transform = Mat4x4f();
	
	model.nodeCount += 1;

	modelNode.parent = parent;
	modelNode.nextSibling = -1; // This might be updated later

	// If this node has children, first child will have index value nodeCount
	modelNode.firstChild = (node->children_count > 0) ? static_cast<int16_t>(model.nodeCount) : -1;

	uint32_t lastSiblingIndex = 0;
	for (size_t i = 0; i < node->children_count; ++i)
	{
		if (i > 0)
			model.nodes[lastSiblingIndex].nextSibling = model.nodeCount;
		
		lastSiblingIndex = model.nodeCount;

		LoadNode(model, thisNodeIndex, data, node->children[i]);
	}
}

bool ModelLoader::LoadMesh(cgltf_mesh* cgltfMesh, ModelMesh& modelMeshOut)
{
	if (cgltfMesh->primitives_count == 0)
		return false;

	cgltf_primitive& prim = cgltfMesh->primitives[0];

	static const uint32_t MaxAttributeCount = 10;
	VertexAttribute attributes[MaxAttributeCount];
	unsigned int attributeCount = 0;

	cgltf_buffer* cgltfVertexBuffer = nullptr;
	size_t vertexCount = 0;

	BoundingBox meshBounds;

	for (size_t i = 0; i < prim.attributes_count; ++i)
	{
		cgltf_attribute& attr = prim.attributes[i];

		if (attr.type == cgltf_attribute_type_tangent)
			continue;

		cgltf_accessor& accessor = *attr.data;

		// Make sure all vertex attributes come from same buffer
		assert(cgltfVertexBuffer == nullptr || accessor.buffer_view->buffer == cgltfVertexBuffer);
		cgltfVertexBuffer = accessor.buffer_view->buffer;

		// Make sure all attributes have same number of vertices
		assert(vertexCount == 0 || accessor.count == vertexCount);
		vertexCount = accessor.count;

		int componentCount = 0;
		if (accessor.type == cgltf_type_scalar)
			componentCount = 1;
		else if (accessor.type == cgltf_type_vec2)
			componentCount = 2;
		else if (accessor.type == cgltf_type_vec3)
			componentCount = 3;
		else if (accessor.type == cgltf_type_vec4)
			componentCount = 4;
		else
		{
			assert(false && "Unsupported vertex attribute component count");
		}

		int attributePos = 0;
		if (attr.type == cgltf_attribute_type_position)
		{
			attributePos = VertexFormat::AttributeIndexPos;

			assert(accessor.has_min && accessor.has_max);

			Vec3f min(accessor.min[0], accessor.min[1], accessor.min[2]);
			Vec3f max(accessor.max[0], accessor.max[1], accessor.max[2]);
			
			meshBounds.center = (min + max) * 0.5f;
			meshBounds.extents = (max - min) * 0.5f;
		}
		else if (attr.type == cgltf_attribute_type_normal)
			attributePos = VertexFormat::AttributeIndexNor;
		else if (attr.type == cgltf_attribute_type_texcoord)
			attributePos = VertexFormat::AttributeIndexUV0;
		else if (attr.type == cgltf_attribute_type_color)
			attributePos = VertexFormat::AttributeIndexCol0;

		RenderVertexElemType componentType = RenderVertexElemType::Float;
		assert(accessor.component_type == cgltf_component_type_r_32f);

		attributes[attributeCount].attrIndex = attributePos;
		attributes[attributeCount].elemCount = componentCount;
		attributes[attributeCount].offset = accessor.offset + accessor.buffer_view->offset;
		attributes[attributeCount].stride = accessor.stride;
		attributes[attributeCount].elemType = componentType;

		attributeCount += 1;
	}

	VertexFormat format(attributes, attributeCount);

	RenderPrimitiveMode primitiveMode = RenderPrimitiveMode::Triangles;
	if (prim.type == cgltf_primitive_type_triangles)
		primitiveMode = RenderPrimitiveMode::Triangles;
	else if (prim.type == cgltf_primitive_type_triangle_fan)
		primitiveMode = RenderPrimitiveMode::TriangleFan;
	else if (prim.type == cgltf_primitive_type_triangle_strip)
		primitiveMode = RenderPrimitiveMode::TriangleStrip;
	else if (prim.type == cgltf_primitive_type_lines)
		primitiveMode = RenderPrimitiveMode::Lines;
	else if (prim.type == cgltf_primitive_type_line_loop)
		primitiveMode = RenderPrimitiveMode::LineLoop;
	else if (prim.type == cgltf_primitive_type_line_strip)
		primitiveMode = RenderPrimitiveMode::LineStrip;
	else if (prim.type == cgltf_primitive_type_points)
		primitiveMode = RenderPrimitiveMode::Points;

	MeshId meshId = modelManager->meshManager->CreateMesh();
	modelManager->meshManager->SetUid(meshId, uid);
	modelManager->meshManager->SetBoundingBox(meshId, meshBounds);

	assert(cgltfVertexBuffer != nullptr);

	if (prim.indices != nullptr)
	{
		uint8_t* indexBufferStart = static_cast<uint8_t*>(prim.indices->buffer_view->buffer->data);
		uint8_t* indexBuffer = indexBufferStart + prim.indices->offset + prim.indices->buffer_view->offset;
		size_t indexBufferSize = prim.indices->buffer_view->size - prim.indices->offset;

		IndexedVertexData indexedVertexData;
		indexedVertexData.vertexFormat = format;
		indexedVertexData.primitiveMode = primitiveMode;
		indexedVertexData.vertexData = cgltfVertexBuffer->data;
		indexedVertexData.vertexDataSize = cgltfVertexBuffer->size;
		indexedVertexData.vertexCount = static_cast<int>(vertexCount);
		indexedVertexData.indexData = indexBuffer;
		indexedVertexData.indexDataSize = indexBufferSize;
		indexedVertexData.indexCount = prim.indices->count;

		modelManager->meshManager->UploadIndexed(meshId, indexedVertexData);
	}
	else
	{
		VertexData nonindexedVertexData;
		nonindexedVertexData.vertexFormat = format;
		nonindexedVertexData.primitiveMode = primitiveMode;
		nonindexedVertexData.vertexData = cgltfVertexBuffer->data;
		nonindexedVertexData.vertexDataSize = cgltfVertexBuffer->size;
		nonindexedVertexData.vertexCount = vertexCount;

		modelManager->meshManager->Upload(meshId, nonindexedVertexData);
	}

	modelMeshOut.meshId = meshId;

	if (cgltfMesh->name != nullptr)
	{
		char* nameBuf = textBuffer.GetData();
		size_t bytesUsed = StringCopyN(nameBuf, cgltfMesh->name, textBuffer.GetCount());

		modelMeshOut.name = nameBuf;
		textBuffer.TrimBeginning(bytesUsed);
	}
	else
		modelMeshOut.name = nullptr;

	return true;
}

}
