#include "Resources/ModelLoader.hpp"

#include <cassert>

#include "cgltf/cgltf.h"

#include "Core/CString.hpp"
#include "Core/SortedArray.hpp"

#include "Memory/Allocator.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/ModelManager.hpp"

namespace
{

struct CountResult
{
	size_t nodeCount = 0;
	size_t meshCount = 0;
	size_t primitiveCount = 0;
	size_t totalStringLength = 0;
	size_t totalGeometryBytes = 0;
};

void CountModelNodeResources(CountResult& resultInOut, cgltf_node* node)
{
	resultInOut.nodeCount += 1;

	for (size_t i = 0; i < node->children_count; ++i)
		CountModelNodeResources(resultInOut, node->children[i]);
}

CountResult CountModelResources(cgltf_data* model, kokko::SortedArray<cgltf_buffer_view*> uniqueGeometryBufferViews)
{
	cgltf_scene& scene = model->scenes[0];

	CountResult result;
	for (size_t i = 0; i < scene.nodes_count; ++i)
		CountModelNodeResources(result, scene.nodes[i]);

	for (size_t mi = 0; mi < model->meshes_count; ++mi)
	{
		cgltf_mesh* mesh = &model->meshes[mi];

		result.meshCount += 1;
		result.primitiveCount += mesh->primitives_count;
		result.totalStringLength += strlen(mesh->name) + 1;

		for (size_t pi = 0; pi < mesh->primitives_count; ++pi)
		{
			cgltf_primitive* prim = &mesh->primitives[pi];
			if (prim->indices != nullptr)
				uniqueGeometryBufferViews.InsertUnique(prim->indices->buffer_view);

			for (size_t vai = 0; vai < prim->attributes_count; ++vai)
			{
				cgltf_attribute* attr = &prim->attributes[vai];
				uniqueGeometryBufferViews.InsertUnique(attr->data->buffer_view);
			}
		}
	}

	for (uint32_t bvi = 0, bvc = uniqueGeometryBufferViews.GetCount(); bvi < bvc; ++bvi)
	{
		cgltf_buffer_view* bufferView = uniqueGeometryBufferViews[bvi];
		result.totalGeometryBytes += bufferView->size;
	}

	return result;
}
/*
void UpdateBuffersIndexed(const kokko::MeshVertexData& vdata)
{
	// Update buffers

	if (bufferData.vertexArrayObject == 0)
	{
		// Create vertex array object
		renderDevice->CreateVertexArrays(1, &bufferData.vertexArrayObject);

		// Create buffer objects
		renderDevice->CreateBuffers(2, bufferData.bufferObjects);

		// Bind and upload index buffer
		renderDevice->SetBufferStorage(bufferData.bufferObjects[MeshBufferData::IndexBuffer],
			vdata.indexDataSize, vdata.indexData, BufferStorageFlags::Dynamic);
		bufferData.bufferSizes[MeshBufferData::IndexBuffer] = vdata.indexDataSize;

		// Bind and upload vertex buffer
		renderDevice->SetBufferStorage(bufferData.bufferObjects[MeshBufferData::VertexBuffer],
			vdata.vertexDataSize, vdata.vertexData, BufferStorageFlags::Dynamic);
		bufferData.bufferSizes[MeshBufferData::VertexBuffer] = vdata.vertexDataSize;
	}
	else
	{
		if (vdata.vertexDataSize <= bufferData.bufferSizes[MeshBufferData::IndexBuffer])
		{
			// Only update the part of the buffer we need
			renderDevice->SetBufferSubData(bufferData.bufferObjects[MeshBufferData::IndexBuffer],
				0, vdata.indexDataSize, vdata.indexData);
		}
		else
		{
			// Recreate buffer and upload data

			renderDevice->DestroyBuffers(1, &bufferData.bufferObjects[MeshBufferData::IndexBuffer]);
			renderDevice->CreateBuffers(1, &bufferData.bufferObjects[MeshBufferData::IndexBuffer]);

			renderDevice->SetBufferStorage(bufferData.bufferObjects[MeshBufferData::IndexBuffer],
				vdata.indexDataSize, vdata.indexData, BufferStorageFlags::Dynamic);

			bufferData.bufferSizes[MeshBufferData::IndexBuffer] = vdata.indexDataSize;
		}

		if (vdata.vertexDataSize <= bufferData.bufferSizes[MeshBufferData::VertexBuffer])
		{
			// Only update the part of the buffer we need
			renderDevice->SetBufferSubData(bufferData.bufferObjects[MeshBufferData::VertexBuffer],
				0, vdata.vertexDataSize, vdata.vertexData);
		}
		else
		{
			// Recreate buffer and upload data

			renderDevice->DestroyBuffers(1, &bufferData.bufferObjects[MeshBufferData::VertexBuffer]);
			renderDevice->CreateBuffers(1, &bufferData.bufferObjects[MeshBufferData::VertexBuffer]);

			renderDevice->SetBufferStorage(bufferData.bufferObjects[MeshBufferData::VertexBuffer],
				vdata.vertexDataSize, vdata.vertexData, BufferStorageFlags::Dynamic);

			bufferData.bufferSizes[MeshBufferData::VertexBuffer] = vdata.vertexDataSize;
		}
	}

	// Draw data

	assert(vdata.vertexCount != 0);

	MeshDrawData& drawData = data.drawData[index];
	drawData.primitiveMode = vdata.primitiveMode;
	drawData.vertexArrayObject = data.bufferData[index].vertexArrayObject;
	drawData.count = static_cast<int>(vdata.indexCount);
	drawData.indexType = vdata.indexType;

	data.uniqueVertexCount[index] = static_cast<int>(vdata.vertexCount);

	// Vertex attributes

	assert(vertexFormat.attributes != nullptr && vertexFormat.attributeCount > 0);

	MeshBufferData& bufferData = data.bufferData[index];
	kokko::render::VertexArrayId vertexArray = bufferData.vertexArrayObject;

	renderDevice->SetVertexArrayIndexBuffer(vertexArray, bufferData.bufferObjects[MeshBufferData::IndexBuffer]);

	for (unsigned int i = 0; i < vertexFormat.attributeCount; ++i)
	{
		const VertexAttribute& attr = vertexFormat.attributes[i];

		renderDevice->SetVertexArrayVertexBuffer(
			vertexArray, i, bufferData.bufferObjects[MeshBufferData::VertexBuffer], attr.offset, attr.stride);
		renderDevice->EnableVertexAttribute(vertexArray, attr.attrIndex);
		renderDevice->SetVertexAttribBinding(vertexArray, attr.attrIndex, i);
		renderDevice->SetVertexAttribFormat(vertexArray, attr.attrIndex, attr.elemCount, attr.elemType, 0);
	}
}
*/

} // namespace

namespace kokko
{

ModelLoader::ModelLoader(Allocator* allocator, ModelManager* modelManager) :
	allocator(allocator),
	modelManager(modelManager),
	uniqueGeometryBufferViews(allocator),
	geometryBufferViewRangeMap(allocator)
{
}

bool ModelLoader::LoadFromBuffer(ModelData* modelOut, ArrayView<const uint8_t> buffer)
{
	outputModel = modelOut;

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

	CountResult countResult = CountModelResources(data, uniqueGeometryBufferViews);

	if (countResult.nodeCount == 0 || countResult.meshCount == 0)
	{
		KK_LOG_ERROR("ModelLoader: Failed to load model, glTF didn't contain any meshes");
		cgltf_free(data);
		return false;
	}

	if (countResult.meshCount > 1) {
		KK_LOG_INFO("ModelLoader: MULTI_MESH");
	}

	if (countResult.primitiveCount > 1) {
		KK_LOG_INFO("ModelLoader: MULTI_PRIMITIVE");
	}

	geometryBufferBytes = countResult.totalGeometryBytes;
	geometryBuffer = allocator->Allocate(geometryBufferBytes, "ModelLoader geometryBuffer");

	// Copy geometry buffer views into one buffer

	geometryBufferViewRangeMap.Reserve(uniqueGeometryBufferViews.GetCount());

	for (size_t i = 0, count = uniqueGeometryBufferViews.GetCount(); i < count; ++i) {
		cgltf_buffer_view* bufferView = uniqueGeometryBufferViews[i];

		assert(geometryBufferUsed + bufferView->size <= geometryBufferBytes);

		uint8_t* dest = static_cast<uint8_t*>(geometryBuffer) + geometryBufferUsed;
		memcpy(dest, bufferView->data, bufferView->size);

		geometryBufferViewRangeMap.PushBack(Range<size_t>(geometryBufferUsed, geometryBufferUsed + bufferView->size));
		geometryBufferUsed += bufferView->size;
	}

	// Allocate model info buffers

	constexpr size_t alignment = 16;
	const size_t nodeBytes = Math::RoundUpToMultiple(sizeof(ModelNode) * countResult.nodeCount, alignment);
	const size_t meshBytes = Math::RoundUpToMultiple(sizeof(ModelMesh) * countResult.meshCount, alignment);
	const size_t primBytes = Math::RoundUpToMultiple(sizeof(ModelPrimitive) * countResult.primitiveCount, alignment);
	const size_t strBytes = countResult.totalStringLength;
	const size_t infoBytes = nodeBytes + meshBytes + primBytes + strBytes;

	outputModel->buffer = allocator->Allocate(infoBytes, "ModelLoader model.buffer");
	uint8_t* byteBuffer = static_cast<uint8_t*>(outputModel->buffer);

	textBuffer = ArrayView<char>(static_cast<char*>(outputModel->buffer) + nodeBytes + meshBytes + primBytes, strBytes);

	outputModel->nodes = reinterpret_cast<ModelNode*>(byteBuffer);
	outputModel->meshes = reinterpret_cast<ModelMesh*>(byteBuffer + nodeBytes);
	outputModel->primitives = reinterpret_cast<ModelPrimitive*>(byteBuffer + nodeBytes + meshBytes);

	// These are used as counters to know how many have been inserted
	outputModel->nodeCount = 0; 
	outputModel->meshCount = 0;
	outputModel->primitiveCount = 0;

	// Load meshes

	for (size_t i = 0, count = data->meshes_count; i < count; ++i)
	{
		cgltf_mesh* cgltfMesh = &data->meshes[i];
		size_t meshIndex = outputModel->meshCount;
		ModelMesh& modelMesh = outputModel->meshes[meshIndex];
		if (LoadMesh(cgltfMesh, modelMesh))
		{
			outputModel->meshCount += 1;
		}
	}

	// Load nodes

	uint32_t lastSiblingIndex = 0;
	cgltf_scene* scene = &data->scenes[0];
	for (size_t i = 0; i < scene->nodes_count; ++i)
	{
		if (i > 0)
			outputModel->nodes[lastSiblingIndex].nextSibling = outputModel->nodeCount;

		lastSiblingIndex = outputModel->nodeCount;

		LoadNode(-1, data, scene->nodes[i]);
	}

	cgltf_free(data);

	outputModel = nullptr;
	textBuffer = ArrayView<char>();
	geometryBuffer = nullptr;
	geometryBufferBytes = 0;
	geometryBufferUsed = 0;
	uniqueGeometryBufferViews.Clear();
	geometryBufferViewRangeMap.Clear();

	return true;
}

void ModelLoader::LoadNode(
	int16_t parent,
	cgltf_data* data,
	cgltf_node* node)
{
	int16_t thisNodeIndex = static_cast<int16_t>(outputModel->nodeCount);
	ModelNode& modelNode = outputModel->nodes[outputModel->nodeCount];

	modelNode.meshIndex = -1;
	if (node->mesh != nullptr)
	{
		cgltf_mesh* meshesEnd = data->meshes + data->meshes_count;
		cgltf_mesh* foundMesh = std::find(data->meshes, meshesEnd, node->mesh);
		if (foundMesh != meshesEnd)
			modelNode.meshIndex = static_cast<int16_t>(foundMesh - data->meshes);
	}

	if (node->has_matrix)
		std::memcpy(&(modelNode.transform), &(node->matrix), sizeof(float) * 16);
	else
		modelNode.transform = Mat4x4f();
	
	outputModel->nodeCount += 1;

	modelNode.parent = parent;
	modelNode.nextSibling = -1; // This might be updated later

	// If this node has children, first child will have index value nodeCount
	modelNode.firstChild = (node->children_count > 0) ? static_cast<int16_t>(outputModel->nodeCount) : -1;

	uint32_t lastSiblingIndex = 0;
	for (size_t i = 0; i < node->children_count; ++i)
	{
		if (i > 0)
			outputModel->nodes[lastSiblingIndex].nextSibling = outputModel->nodeCount;
		
		lastSiblingIndex = outputModel->nodeCount;

		LoadNode(thisNodeIndex, data, node->children[i]);
	}
}

bool ModelLoader::LoadMesh(cgltf_mesh* cgltfMesh, ModelMesh& modelMeshOut)
{
	if (cgltfMesh->primitives_count == 0)
		return false;

	const size_t primitiveCount = cgltfMesh->primitives_count;
	const size_t primitiveOffset = outputModel->primitiveCount;

	modelMeshOut.primitiveCount = 0;
	modelMeshOut.primitiveOffset = primitiveOffset;
	modelMeshOut.name = nullptr;

	cgltf_buffer* cgltfVertexBuffer = nullptr;
	cgltf_buffer* cgltfIndexBuffer = nullptr;

	size_t vertexBufferStartOffset = 0;
	size_t vertexBufferEndOffset = 0;
	size_t indexBufferStartOffset = 0;
	size_t indexBufferEndOffset = 0;

	static const uint32_t MaxAttributeCount = 10;
	VertexAttribute attributes[MaxAttributeCount];
	uint32_t attributeCount = 0;

	Vec3f boundsMin;
	Vec3f boundsMax;

	RenderIndexType indexType = RenderIndexType::None;
	RenderPrimitiveMode primitiveMode = RenderPrimitiveMode::Triangles;

	// A primitive is a separate part of a mesh
	// Using multiple materials would create multiple primitives

	// Iterate through each primitive, once to gather info about attributes and buffer usage
	for (size_t primIdx = 0; primIdx < primitiveCount; ++primIdx)
	{
		const cgltf_primitive& cgltfPrim = cgltfMesh->primitives[primIdx];

		size_t vertexCount = 0;

		// TODO: Make sure all gltf primitives have same mode or allow multiple to be used
		if (cgltfPrim.type == cgltf_primitive_type_triangles)
			primitiveMode = RenderPrimitiveMode::Triangles;
		else if (cgltfPrim.type == cgltf_primitive_type_triangle_fan)
			primitiveMode = RenderPrimitiveMode::TriangleFan;
		else if (cgltfPrim.type == cgltf_primitive_type_triangle_strip)
			primitiveMode = RenderPrimitiveMode::TriangleStrip;
		else if (cgltfPrim.type == cgltf_primitive_type_lines)
			primitiveMode = RenderPrimitiveMode::Lines;
		else if (cgltfPrim.type == cgltf_primitive_type_line_loop)
			primitiveMode = RenderPrimitiveMode::LineLoop;
		else if (cgltfPrim.type == cgltf_primitive_type_line_strip)
			primitiveMode = RenderPrimitiveMode::LineStrip;
		else if (cgltfPrim.type == cgltf_primitive_type_points)
			primitiveMode = RenderPrimitiveMode::Points;

		// Indices

		size_t indexCount = 0;
		if (cgltfPrim.indices != nullptr)
		{
			if (cgltfIndexBuffer == nullptr)
			{
				cgltfIndexBuffer = cgltfPrim.indices->buffer_view->buffer;
				indexBufferStartOffset = cgltfIndexBuffer->size;
				indexCount = cgltfPrim.indices->count;
			}
			else
			{
				// Make sure indices for all primitives come from same buffer
				assert(cgltfPrim.indices->buffer_view->buffer == cgltfIndexBuffer);
			}

			RenderIndexType primIndexType = RenderIndexType::None;
			size_t indexSize = 0;
			if (cgltfPrim.indices->component_type == cgltf_component_type_r_32u)
			{
				primIndexType = RenderIndexType::UnsignedInt;
				indexSize = 4;
			}
			else if (cgltfPrim.indices->component_type == cgltf_component_type_r_16u)
			{
				primIndexType = RenderIndexType::UnsignedShort;
				indexSize = 2;
			}
			else if (cgltfPrim.indices->component_type == cgltf_component_type_r_8u)
			{
				primIndexType = RenderIndexType::UnsignedByte;
				indexSize = 1;
			}
			else
				assert(false && "Unsupported index type");

			if (indexType != RenderIndexType::None)
				assert(indexType == primIndexType);

			indexType = primIndexType;

			size_t indicesStart = cgltfPrim.indices->offset + cgltfPrim.indices->buffer_view->offset;
			size_t indicesEnd = indicesStart + cgltfPrim.indices->count * indexSize;
			indexBufferStartOffset = std::min(indexBufferStartOffset, indicesStart);
			indexBufferEndOffset = std::max(indexBufferEndOffset, indicesEnd);
		}

		// Vertex attributes

		for (size_t i = 0; i < cgltfPrim.attributes_count; ++i)
		{
			cgltf_attribute& attr = cgltfPrim.attributes[i];

			if (attr.type == cgltf_attribute_type_tangent)
				continue;

			cgltf_accessor& accessor = *attr.data;

			if (cgltfVertexBuffer == nullptr)
			{
				cgltfVertexBuffer = accessor.buffer_view->buffer;
				vertexBufferStartOffset = cgltfVertexBuffer->size;
				vertexCount = accessor.count;
			}
			else
			{
				// Make sure all vertex attributes come from same buffer
				assert(accessor.buffer_view->buffer == cgltfVertexBuffer);

				// Make sure all attributes have same number of vertices
				assert(accessor.count == vertexCount);
			}

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
				assert(false && "Unsupported vertex attribute component count");

			int attributePos = 0;
			if (attr.type == cgltf_attribute_type_position)
			{
				attributePos = VertexFormat::AttributeIndexPos;

				assert(accessor.has_min && accessor.has_max);

				boundsMin.x = std::min(boundsMin.x, accessor.min[0]);
				boundsMin.y = std::min(boundsMin.y, accessor.min[1]);
				boundsMin.z = std::min(boundsMin.z, accessor.min[2]);
				boundsMax.x = std::max(boundsMax.x, accessor.max[0]);
				boundsMax.y = std::max(boundsMax.y, accessor.max[1]);
				boundsMax.z = std::max(boundsMax.z, accessor.max[2]);
			}
			else if (attr.type == cgltf_attribute_type_normal)
				attributePos = VertexFormat::AttributeIndexNor;
			else if (attr.type == cgltf_attribute_type_texcoord)
				attributePos = VertexFormat::AttributeIndexUV0;
			else if (attr.type == cgltf_attribute_type_color)
				attributePos = VertexFormat::AttributeIndexCol0;

			RenderVertexElemType componentType = RenderVertexElemType::Float;
			assert(accessor.component_type == cgltf_component_type_r_32f);

			size_t attributeStart = accessor.offset + accessor.buffer_view->offset;
			size_t attributeEnd = attributeStart + accessor.count * accessor.stride;
			vertexBufferStartOffset = std::min(vertexBufferStartOffset, attributeStart);
			vertexBufferEndOffset = std::max(vertexBufferEndOffset, attributeEnd);

			attributes[attributeCount].attrIndex = attributePos;
			attributes[attributeCount].elemCount = componentCount;
			attributes[attributeCount].offset = attributeStart;
			attributes[attributeCount].stride = accessor.stride;
			attributes[attributeCount].elemType = componentType;

			attributeCount += 1;
		}
		
		ModelPrimitive& modelPrim = outputModel->primitives[primitiveOffset + primIdx];
		modelPrim.count = cgltfPrim.indices != nullptr ? cgltfPrim.indices->count : vertexCount;
		modelPrim.indexOffset = 0;
		modelPrim.vertexOffset = 0;
	}

	AABB meshBounds;
	meshBounds.center = (boundsMin + boundsMax) * 0.5f;
	meshBounds.extents = (boundsMax - boundsMin) * 0.5f;

	// Iterate primitives a second time to calculate accurate buffer offsets
	for (size_t primIdx = 0; primIdx < primitiveCount; ++primIdx)
	{
		for (size_t attrIdx = 0; attrIdx < attributeCount; ++attrIdx)
		{
			attributes[attrIdx].offset -= vertexBufferStartOffset;
		}

		const cgltf_primitive& cgltfPrim = cgltfMesh->primitives[primIdx];

		outputModel->primitiveCount += 1;
		modelMeshOut.primitiveCount += 1;

		ModelPrimitive& modelPrim = outputModel->primitives[primitiveOffset + primIdx];

		if (cgltfPrim.indices != nullptr)
		{
			size_t primitiveIndicesOffset = cgltfPrim.indices->buffer_view->offset + cgltfPrim.indices->offset;
			modelPrim.indexOffset = primitiveIndicesOffset - indexBufferStartOffset;
		}
	}

	assert(cgltfVertexBuffer != nullptr);

	// NONIIN nyt sain jotain etenem‰‰n kun unohdin vain aikasemmat koodit ja alotin v‰h‰ tyhj‰lt‰
	// Seuraavaksi vaan mietin miten s‰ilyt‰n geometriatiedon ModelDatassa

	VertexFormat format(attributes, attributeCount);

	MeshVertexData vertexDataInfo;
	vertexDataInfo.vertexFormat = format;
	vertexDataInfo.primitiveMode = primitiveMode;
	vertexDataInfo.vertexData = static_cast<const uint8_t*>(cgltfVertexBuffer->data) + vertexBufferStartOffset;
	vertexDataInfo.vertexDataSize = vertexBufferEndOffset - vertexBufferStartOffset;
	vertexDataInfo.vertexCount = 3;

	if (cgltfIndexBuffer != nullptr)
	{
		uint8_t* indexBuffer = static_cast<uint8_t*>(cgltfIndexBuffer->data) + indexBufferStartOffset;
		size_t indexBufferSize = indexBufferEndOffset - indexBufferStartOffset;

		vertexDataInfo.indexData = indexBuffer;
		vertexDataInfo.indexDataSize = indexBufferSize;
		vertexDataInfo.indexCount = 3;
		vertexDataInfo.indexType = indexType;
	}
	else
	{
		VertexData vertexData;
		vertexData.vertexFormat = format;
		vertexData.primitiveMode = primitiveMode;
		vertexData.vertexData = static_cast<const uint8_t*>(cgltfVertexBuffer->data) + vertexBufferStartOffset;
		vertexData.vertexDataSize = vertexBufferEndOffset - vertexBufferStartOffset;
		vertexData.vertexCount = 3;
	}

	//UpdateBuffersIndexed(index, vertexData);

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


} // namespace kokko
