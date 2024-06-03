#include "Resources/ModelLoader.hpp"

#include <cassert>

#include "cgltf/cgltf.h"
#include "doctest/doctest.h"

#include "Core/CString.hpp"
#include "Core/SortedArray.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/VertexFormat.hpp"

#include "Resources/ModelManager.hpp"

#include "System/Filesystem.hpp"

namespace
{

struct CountResult
{
	size_t nodeCount = 0;
	size_t meshCount = 0;
	size_t meshPartCount = 0;
	size_t attributeCount = 0;
	size_t totalStringLength = 0;
	size_t totalGeometryBytes = 0;
};

void CountModelNodeResources(CountResult& resultInOut, cgltf_node* node)
{
	resultInOut.nodeCount += 1;

	for (size_t i = 0; i < node->children_count; ++i)
		CountModelNodeResources(resultInOut, node->children[i]);
}

CountResult CountModelResources(cgltf_data* model, kokko::SortedArray<cgltf_buffer_view*>& uniqueGeometryBufferViews)
{
	cgltf_scene& scene = model->scenes[0];

	CountResult result;
	for (size_t i = 0; i < scene.nodes_count; ++i)
		CountModelNodeResources(result, scene.nodes[i]);

	for (size_t mi = 0; mi < model->meshes_count; ++mi)
	{
		cgltf_mesh* mesh = &model->meshes[mi];

		result.meshCount += 1;
		result.meshPartCount += mesh->primitives_count;
		result.totalStringLength += strlen(mesh->name) + 1;

		for (size_t pi = 0; pi < mesh->primitives_count; ++pi)
		{
			cgltf_primitive* cgltfPrim = &mesh->primitives[pi];

			result.attributeCount += cgltfPrim->attributes_count;

			if (cgltfPrim->indices != nullptr)
				uniqueGeometryBufferViews.InsertUnique(cgltfPrim->indices->buffer_view);

			for (size_t vai = 0; vai < cgltfPrim->attributes_count; ++vai)
			{
				cgltf_attribute* attr = &cgltfPrim->attributes[vai];
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

} // namespace

namespace kokko
{

ModelLoader::ModelLoader(Allocator* allocator) :
	allocator(allocator),
	uniqueGeometryBufferViews(allocator),
	geometryBufferViewRangeMap(allocator)
{
}

bool ModelLoader::LoadRuntime(ModelData* modelOut, Array<uint8_t>* geometryBufferOut, const ModelCreateInfo& createInfo)
{
	KOKKO_PROFILE_FUNCTION();

	outputModel = modelOut;
	geometryBuffer = geometryBufferOut;
	
	constexpr size_t alignment = 8;
	const size_t vertexBytes = Math::RoundUpToMultiple(createInfo.vertexDataSize, alignment);
	const size_t indexBytes = Math::RoundUpToMultiple(createInfo.indexDataSize, alignment);
	const size_t geometryBufferBytes = vertexBytes + indexBytes;
	geometryBuffer->Resize(geometryBufferBytes);

	// Copy geometry buffers into one buffer

	memcpy(geometryBuffer->GetData(), createInfo.vertexData, createInfo.vertexDataSize);
	if (createInfo.indexType != RenderIndexType::None)
		memcpy(geometryBuffer->GetData() + vertexBytes, createInfo.indexData, createInfo.indexDataSize);

	// Allocate model info buffers

	const size_t meshBytes = Math::RoundUpToMultiple(sizeof(ModelMesh), alignment);
	const size_t partBytes = Math::RoundUpToMultiple(sizeof(ModelMeshPart), alignment);
	const size_t attrBytes = Math::RoundUpToMultiple(sizeof(VertexAttribute) * createInfo.vertexFormat.attributeCount, alignment);
	const size_t infoBytes = meshBytes + partBytes + attrBytes;

	outputModel->buffer = allocator->Allocate(infoBytes, "ModelLoader model.buffer");
	uint8_t* byteBuffer = static_cast<uint8_t*>(outputModel->buffer);

	outputModel->nodes = nullptr;
	outputModel->meshes = reinterpret_cast<ModelMesh*>(byteBuffer);
	outputModel->meshParts = reinterpret_cast<ModelMeshPart*>(byteBuffer + meshBytes);
	outputModel->attributes = reinterpret_cast<VertexAttribute*>(byteBuffer + meshBytes + partBytes);

	ModelMesh& mesh = outputModel->meshes[0];
	mesh.partCount = 1;
	mesh.partOffset = 0;
	mesh.indexType = createInfo.indexType;
	mesh.primitiveMode = createInfo.primitiveMode;
	mesh.name = nullptr;
	mesh.aabb = AABB();

	ModelMeshPart& meshPart = outputModel->meshParts[0];
	meshPart.uniqueVertexCount = createInfo.vertexCount;
	meshPart.indexOffset = vertexBytes;
	meshPart.count = createInfo.indexType != RenderIndexType::None ? createInfo.indexCount : createInfo.vertexCount;
	meshPart.vertexFormat.attributes = outputModel->attributes;
	meshPart.vertexFormat.attributeCount = createInfo.vertexFormat.attributeCount;

	for (uint32_t attrIdx = 0; attrIdx < createInfo.vertexFormat.attributeCount; ++attrIdx)
	{
		const VertexAttribute& attributeIn = createInfo.vertexFormat.attributes[attrIdx];
		outputModel->attributes[attrIdx] = attributeIn;
	}

	outputModel->nodeCount = 0;
	outputModel->meshCount = 1;
	outputModel->meshPartCount = 1;
	outputModel->attributeCount = createInfo.vertexFormat.attributeCount;

	Reset();

    return true;
}

bool ModelLoader::LoadGlbFromBuffer(
	ModelData *modelOut,
	Array<uint8_t> *geometryBufferOut,
	ArrayView<const uint8_t> buffer)
{
	/*
	1. Count all model resources
	2. Allocate all required memory for model info in one buffer
	3. Allocate all required memory for geometry in another buffer
		- This is because it will be released as soon as the data is uploaded to the GPU
	3. Copy all geometry data into the buffer
		- Keep track of the buffer views mapping in the buffer
	4. Load each mesh in model
		- Load each glTF primitive in mesh
			- glTF primitives are called mesh parts in code to reduce confusion with the rendering API term primitive,
			  which means triangle, point, line, etc.
			- Mesh parts each have their own vertex format and vertex array for now. In the future, we should try to
			  merge these when possible to make it possible to render all parts in one draw call.
			- Each vertex attribute can come from separate buffer view
	5. Load each node in model
	*/

	KOKKO_PROFILE_FUNCTION();

	outputModel = modelOut;
	geometryBuffer = geometryBufferOut;

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
	 
	size_t geometryBufferBytes = countResult.totalGeometryBytes;
	geometryBuffer->Resize(geometryBufferBytes);

	// Copy geometry buffer views into one buffer

	geometryBufferViewRangeMap.Reserve(uniqueGeometryBufferViews.GetCount());

	for (size_t i = 0, count = uniqueGeometryBufferViews.GetCount(); i < count; ++i) {
		cgltf_buffer_view* bufferView = uniqueGeometryBufferViews[i];

		assert(geometryBufferUsed + bufferView->size <= geometryBufferBytes);

		uint8_t* dest = geometryBuffer->GetData() + geometryBufferUsed;
		const uint8_t* bufferData = static_cast<const uint8_t*>(bufferView->buffer->data);
		memcpy(dest, bufferData + bufferView->offset, bufferView->size);

		geometryBufferViewRangeMap.PushBack(Range<size_t>(geometryBufferUsed, geometryBufferUsed + bufferView->size));
		geometryBufferUsed += bufferView->size;
	}

	// Allocate model info buffers

	constexpr size_t alignment = 8;
	const size_t nodeBytes = Math::RoundUpToMultiple(sizeof(ModelNode) * countResult.nodeCount, alignment);
	const size_t meshBytes = Math::RoundUpToMultiple(sizeof(ModelMesh) * countResult.meshCount, alignment);
	const size_t partBytes = Math::RoundUpToMultiple(sizeof(ModelMeshPart) * countResult.meshPartCount, alignment);
	const size_t attrBytes = Math::RoundUpToMultiple(sizeof(VertexAttribute) * countResult.attributeCount, alignment);
	const size_t strBytes = countResult.totalStringLength;
	const size_t infoBytes = nodeBytes + meshBytes + partBytes + attrBytes + strBytes;

	outputModel->buffer = allocator->Allocate(infoBytes, "ModelLoader model.buffer");
	uint8_t* byteBuffer = static_cast<uint8_t*>(outputModel->buffer);

	textBuffer = ArrayView<char>(static_cast<char*>(outputModel->buffer) + nodeBytes + meshBytes + partBytes + attrBytes, strBytes);

	outputModel->nodes = reinterpret_cast<ModelNode*>(byteBuffer);
	outputModel->meshes = reinterpret_cast<ModelMesh*>(byteBuffer + nodeBytes);
	outputModel->meshParts = reinterpret_cast<ModelMeshPart*>(byteBuffer + nodeBytes + meshBytes);
	outputModel->attributes = reinterpret_cast<VertexAttribute*>(byteBuffer + nodeBytes + meshBytes + partBytes);

	// These are used as counters to know how many have been inserted
	outputModel->nodeCount = 0;
	outputModel->meshCount = 0;
	outputModel->meshPartCount = 0;
	outputModel->attributeCount = 0;

	// Load meshes

	for (size_t i = 0, count = data->meshes_count; i < count; ++i)
	{
		cgltf_mesh* cgltfMesh = &data->meshes[i];
		size_t meshIndex = outputModel->meshCount;
		ModelMesh& modelMesh = outputModel->meshes[meshIndex];
		if (LoadGltfMesh(cgltfMesh, modelMesh))
			outputModel->meshCount += 1;
	}

	// Load nodes

	uint32_t lastSiblingIndex = 0;
	cgltf_scene* scene = &data->scenes[0];
	for (size_t i = 0; i < scene->nodes_count; ++i)
	{
		if (i > 0)
			outputModel->nodes[lastSiblingIndex].nextSibling = outputModel->nodeCount;

		lastSiblingIndex = outputModel->nodeCount;

		LoadGltfNode(-1, data, scene->nodes[i]);
	}

	cgltf_free(data);

	Reset();

	return true;
}

// === PRIVATE METHODS ===

void ModelLoader::Reset()
{
	outputModel = nullptr;
	textBuffer = ArrayView<char>();
	geometryBuffer = nullptr;
	geometryBufferUsed = 0;
	uniqueGeometryBufferViews.Clear();
	geometryBufferViewRangeMap.Clear();
}

void ModelLoader::LoadGltfNode(
    int16_t parent,
    cgltf_data *data,
    cgltf_node *node)
{
	int16_t thisNodeIndex = static_cast<int16_t>(outputModel->nodeCount);
	ModelNode& modelNode = outputModel->nodes[outputModel->nodeCount];

	modelNode.meshIndex = -1;
	if (node->mesh != nullptr)
	{
		cgltf_mesh* meshesEnd = data->meshes + data->meshes_count;
		if (node->mesh <= data->meshes && node->mesh < data->meshes + data->meshes_count)
			modelNode.meshIndex = static_cast<int16_t>(node->mesh - data->meshes);
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

		LoadGltfNode(thisNodeIndex, data, node->children[i]);
	}
}

bool ModelLoader::LoadGltfMesh(cgltf_mesh* cgltfMesh, ModelMesh& modelMeshOut)
{
	if (cgltfMesh->primitives_count == 0)
		return false;

	const size_t meshPartCount = cgltfMesh->primitives_count;
	const uint32_t meshPartOffset = outputModel->meshPartCount;

	modelMeshOut.partCount = 0;
	modelMeshOut.partOffset = meshPartOffset;
	modelMeshOut.name = nullptr;

	Vec3f boundsMin;
	Vec3f boundsMax;

	RenderIndexType indexType = RenderIndexType::None;
	RenderPrimitiveMode primitiveMode = RenderPrimitiveMode::Triangles;

	// A glTF primitive is called mesh part in this code
	// Using multiple materials would create multiple mesh parts

	// Iterate through each mesh part to gather info about attributes and buffer usage
	for (size_t partIdx = 0; partIdx < meshPartCount; ++partIdx)
	{
		const cgltf_primitive& cgltfPrim = cgltfMesh->primitives[partIdx];

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
			RenderIndexType partIndexType = RenderIndexType::None;
			size_t indexSize = 0;
			if (cgltfPrim.indices->component_type == cgltf_component_type_r_32u)
			{
				partIndexType = RenderIndexType::UnsignedInt;
				indexSize = 4;
			}
			else if (cgltfPrim.indices->component_type == cgltf_component_type_r_16u)
			{
				partIndexType = RenderIndexType::UnsignedShort;
				indexSize = 2;
			}
			else if (cgltfPrim.indices->component_type == cgltf_component_type_r_8u)
			{
				partIndexType = RenderIndexType::UnsignedByte;
				indexSize = 1;
			}
			else
				assert(false && "Unsupported index type");

			if (indexType != RenderIndexType::None)
				assert(indexType == partIndexType);

			indexType = partIndexType;
		}

		// Vertex attributes

		VertexAttribute* attributesBegin = &outputModel->attributes[outputModel->attributeCount];

		for (size_t i = 0; i < cgltfPrim.attributes_count; ++i)
		{
			cgltf_attribute& attr = cgltfPrim.attributes[i];

			if (attr.type == cgltf_attribute_type_tangent)
				continue;

			cgltf_accessor& accessor = *attr.data;

			if (vertexCount == 0)
				vertexCount = accessor.count;
			else
				assert(accessor.count == vertexCount); // Make sure all attributes have same number of vertices

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

			auto attributeBVIdx = uniqueGeometryBufferViews.Find(accessor.buffer_view);
			if (attributeBVIdx >= 0)
			{
				size_t bufferViewStart = geometryBufferViewRangeMap[attributeBVIdx].start;
				size_t attributeOffset = bufferViewStart + accessor.offset;

				auto& attributeOut = outputModel->attributes[outputModel->attributeCount];
				attributeOut.attrIndex = attributePos;
				attributeOut.elemCount = componentCount;
				attributeOut.offset = attributeOffset;
				attributeOut.stride = accessor.stride;
				attributeOut.elemType = componentType;
				outputModel->attributeCount += 1;
			}
			else
			{
				assert(false && "Vertex attribute geometry buffer view not found");
			}
		}
		
		ModelMeshPart& meshPart = outputModel->meshParts[meshPartOffset + partIdx];
		meshPart.uniqueVertexCount = vertexCount;
		meshPart.count = cgltfPrim.indices != nullptr ? cgltfPrim.indices->count : vertexCount;
		meshPart.indexOffset = 0;

		if (cgltfPrim.indices != nullptr)
		{
			auto indexBufferViewIdx = uniqueGeometryBufferViews.Find(cgltfPrim.indices->buffer_view);
			if (indexBufferViewIdx >= 0)
				meshPart.indexOffset = geometryBufferViewRangeMap[indexBufferViewIdx].start + cgltfPrim.indices->offset;
			else
				assert(false && "Index geometry buffer view not found in range map");
		}

		meshPart.vertexFormat = VertexFormat(attributesBegin, cgltfPrim.attributes_count);

		outputModel->meshPartCount += 1;
		modelMeshOut.partCount += 1;
	}

	modelMeshOut.primitiveMode = primitiveMode;
	modelMeshOut.indexType = indexType;

	modelMeshOut.aabb.center = (boundsMin + boundsMax) * 0.5f;
	modelMeshOut.aabb.extents = (boundsMax - boundsMin) * 0.5f;

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

TEST_CASE("ModelLoader.RuntimeModelNonIndexed")
{
	Allocator* allocator = Allocator::GetDefault();
	VertexAttribute vertexAttributes[] = { VertexAttribute::pos3, VertexAttribute::uv0 };
	VertexFormat vertexFormat(vertexAttributes, KOKKO_ARRAY_ITEMS(vertexAttributes));
	vertexFormat.CalcOffsetsAndSizeInterleaved();
	ModelLoader modelLoader(allocator);

	const float vertexData[] = {
		1.0f, 2.0f, 8.0f, 0.0f, 0.0f,
		3.5f, 2.75f, -1.9625f, 0.0f, 1.0f,
		9.125f, -5.555f, -2.564f, 1.0f, 0.0f,
		2.5f, 5.5f, 3.215f, 1.0f, 0.0f,
		-4.5f, 23.1f, -5.3f, 0.0f, 1.0f,
		2.995f, -0.3f, -0.4312f, 1.0f, 1.0f
	};

	ModelCreateInfo modelInfo;
	modelInfo.vertexFormat = vertexFormat;
	modelInfo.primitiveMode = RenderPrimitiveMode::Triangles;
	modelInfo.vertexData = vertexData;
	modelInfo.vertexDataSize = sizeof(vertexData);
	modelInfo.vertexCount = KOKKO_ARRAY_ITEMS(vertexData) / vertexAttributes[0].stride;

	ModelData model;
	Array<uint8_t> geometryBuffer(allocator);
	CHECK(modelLoader.LoadRuntime(&model, &geometryBuffer, modelInfo) == true);
	CHECK(geometryBuffer.GetCount() >= sizeof(vertexData));

	const float* p = reinterpret_cast<const float*>(geometryBuffer.GetData());
	for (size_t i = 0, count = KOKKO_ARRAY_ITEMS(vertexData); i != count; ++i)
	{
		CHECK(p[i] == vertexData[i]);
	}

	CHECK(model.attributeCount == KOKKO_ARRAY_ITEMS(vertexAttributes));
	// Mesh attributes are not directly accessed through ModelData.attributes
	// So no need to double test them

	CHECK(model.nodeCount == 0);

	CHECK(model.meshCount == 1);
	CHECK(model.meshes[0].indexType == modelInfo.indexType);
	CHECK(model.meshes[0].partCount == 1);
	CHECK(model.meshes[0].partOffset == 0);
	CHECK(model.meshes[0].primitiveMode == modelInfo.primitiveMode);

	CHECK(model.meshPartCount == 1);
	CHECK(model.meshParts[0].count == modelInfo.vertexCount);
	CHECK(model.meshParts[0].uniqueVertexCount == modelInfo.vertexCount);
	CHECK(model.meshParts[0].vertexFormat.attributeCount == KOKKO_ARRAY_ITEMS(vertexAttributes));
	CHECK(model.meshParts[0].vertexFormat.attributes[0].elemCount == vertexAttributes[0].elemCount);
	CHECK(model.meshParts[0].vertexFormat.attributes[0].elemType == vertexAttributes[0].elemType);
}

TEST_CASE("ModelLoader.RuntimeModelIndexed")
{
	Allocator* allocator = Allocator::GetDefault();
	VertexAttribute vertexAttributes[] = { VertexAttribute::pos3, VertexAttribute::uv0 };
	VertexFormat vertexFormat(vertexAttributes, KOKKO_ARRAY_ITEMS(vertexAttributes));
	vertexFormat.CalcOffsetsAndSizeInterleaved();
	ModelLoader modelLoader(allocator);

	const float vertexData[] = {
		1.0f, 2.0f, 8.0f, 0.0f, 0.0f,
		3.5f, 2.75f, -1.9625f, 0.0f, 1.0f,
		9.125f, -5.555f, -2.564f, 1.0f, 0.0f,
		2.5f, 5.5f, 3.215f, 1.0f, 0.0f,
		-4.5f, 23.1f, -5.3f, 0.0f, 1.0f,
		2.995f, -0.3f, -0.4312f, 1.0f, 1.0f
	};

	uint16_t indexData[] = {
		0, 1, 2, 2, 1, 3, 0, 1, 3, 3, 1, 2
	};

	ModelCreateInfo modelInfo;
	modelInfo.vertexFormat = vertexFormat;
	modelInfo.primitiveMode = RenderPrimitiveMode::Triangles;
	modelInfo.vertexData = vertexData;
	modelInfo.vertexDataSize = sizeof(vertexData);
	modelInfo.vertexCount = KOKKO_ARRAY_ITEMS(vertexData) / vertexAttributes[0].stride;
	modelInfo.indexData = indexData;
	modelInfo.indexDataSize = sizeof(indexData);
	modelInfo.indexCount = KOKKO_ARRAY_ITEMS(indexData);
	modelInfo.indexType = RenderIndexType::UnsignedShort;

	ModelData model;
	Array<uint8_t> geometryBuffer(allocator);
	CHECK(modelLoader.LoadRuntime(&model, &geometryBuffer, modelInfo) == true);

	CHECK(model.attributeCount == KOKKO_ARRAY_ITEMS(vertexAttributes));
	// Mesh attributes are not directly accessed through ModelData.attributes
	// So no need to double test them

	CHECK(model.nodeCount == 0);

	CHECK(model.meshCount == 1);
	CHECK(model.meshes[0].indexType == modelInfo.indexType);
	CHECK(model.meshes[0].partCount == 1);
	CHECK(model.meshes[0].partOffset == 0);
	CHECK(model.meshes[0].primitiveMode == modelInfo.primitiveMode);

	CHECK(model.meshPartCount == 1);
	CHECK(model.meshParts[0].count == modelInfo.indexCount);
	CHECK(model.meshParts[0].uniqueVertexCount == modelInfo.vertexCount);
	CHECK(model.meshParts[0].vertexFormat.attributeCount == KOKKO_ARRAY_ITEMS(vertexAttributes));

	for (uint32_t aIdx = 0; aIdx != model.meshParts[0].vertexFormat.attributeCount; ++aIdx)
	{
		CHECK(model.meshParts[0].vertexFormat.attributes[aIdx].elemCount == vertexAttributes[aIdx].elemCount);
		CHECK(model.meshParts[0].vertexFormat.attributes[aIdx].elemType == vertexAttributes[aIdx].elemType);
	}

	CHECK(geometryBuffer.GetCount() >= sizeof(vertexData) + sizeof(indexData));

	const uint8_t* origGeom = reinterpret_cast<const uint8_t*>(vertexData);
	const uint8_t* geom = geometryBuffer.GetData();
	for (uint32_t vIdx = 0; vIdx != model.meshParts[0].uniqueVertexCount; ++vIdx)
	{
		for (uint32_t aIdx = 0; aIdx != model.meshParts[0].vertexFormat.attributeCount; ++aIdx)
		{
			auto& origAttr = vertexAttributes[aIdx];
			auto& attr = model.meshParts[0].vertexFormat.attributes[aIdx];
			auto origData = reinterpret_cast<const float*>(&origGeom[origAttr.offset + attr.stride * vIdx]);
			auto attrData = reinterpret_cast<const float*>(&geom[attr.offset + attr.stride * vIdx]);
			for (uint32_t eIdx = 0; eIdx != attr.elemCount; ++eIdx)
			{
				CHECK(origData[eIdx] == attrData[eIdx]);
			}
		}
	}

	auto indices = reinterpret_cast<const uint16_t*>(&geom[model.meshParts[0].indexOffset]);
	for (uint32_t idx = 0; idx != model.meshParts[0].count; ++idx)
	{
		CHECK(indices[idx] == indexData[idx]);
	}
}

TEST_CASE("ModelLoader.GlbModelIndexed")
{
	Allocator* allocator = Allocator::GetDefault();
	Filesystem filesystem(allocator, nullptr);
	Array<uint8_t> buffer(allocator);
	CHECK(filesystem.ReadBinary("test/res/model/Box.glb", buffer) == true);

	ModelLoader modelLoader(allocator);
	ModelData model;
	Array<uint8_t> geometryBuffer(allocator);
	CHECK(modelLoader.LoadGlbFromBuffer(&model, &geometryBuffer, buffer.GetView()) == true);

	CHECK(model.attributeCount == 2);

	CHECK(model.nodeCount == 2);

	CHECK(model.meshCount == 1);
	CHECK(model.meshes[0].indexType == RenderIndexType::UnsignedShort);
	CHECK(model.meshes[0].partCount == 1);
	CHECK(model.meshes[0].partOffset == 0);
	CHECK(model.meshes[0].primitiveMode == RenderPrimitiveMode::Triangles);
	CHECK(strcmp(model.meshes[0].name, "Mesh") == 0);

	CHECK(model.meshPartCount == 1);
	CHECK(model.meshParts[0].count == 36);
	CHECK(model.meshParts[0].uniqueVertexCount == 24);
	CHECK(model.meshParts[0].vertexFormat.attributeCount == 2);

	CHECK(model.meshParts[0].vertexFormat.attributes[0].elemCount == 3);
	CHECK(model.meshParts[0].vertexFormat.attributes[0].elemType == RenderVertexElemType::Float);
	CHECK(model.meshParts[0].vertexFormat.attributes[0].stride == 12);

	CHECK(model.meshParts[0].vertexFormat.attributes[1].elemCount == 3);
	CHECK(model.meshParts[0].vertexFormat.attributes[1].elemType == RenderVertexElemType::Float);
	CHECK(model.meshParts[0].vertexFormat.attributes[0].stride == 12);

	size_t vertexDataSize = 3 * sizeof(float) * 2 * model.meshParts[0].uniqueVertexCount;
	size_t indexDataSize = 2 * model.meshParts[0].count;
	CHECK(geometryBuffer.GetCount() >= vertexDataSize + indexDataSize);

	ArrayView<const uint8_t> binaryData = buffer.GetSubView(1016, buffer.GetCount());

	// These attributes represent the data in the original glb buffer
	VertexAttribute vertexAttributes[] = { VertexAttribute::nor, VertexAttribute::pos3 };
	vertexAttributes[0].stride = 12;
	vertexAttributes[0].offset = 0;
	vertexAttributes[1].stride = 12;
	vertexAttributes[1].offset = 288;

	const uint8_t* origGeom = binaryData.GetData();
	const uint8_t* geom = geometryBuffer.GetData();
	for (uint32_t vIdx = 0; vIdx != model.meshParts[0].uniqueVertexCount; ++vIdx)
	{
		for (uint32_t aIdx = 0; aIdx != model.meshParts[0].vertexFormat.attributeCount; ++aIdx)
		{
			auto& origAttr = vertexAttributes[aIdx];
			auto& attr = model.meshParts[0].vertexFormat.attributes[aIdx];
			auto origData = reinterpret_cast<const float*>(&origGeom[origAttr.offset + origAttr.stride * vIdx]);
			auto attrData = reinterpret_cast<const float*>(&geom[attr.offset + attr.stride * vIdx]);
			for (uint32_t eIdx = 0; eIdx != attr.elemCount; ++eIdx)
			{
				CHECK(origData[eIdx] == attrData[eIdx]);
			}
		}
	}

	auto originalIndices = reinterpret_cast<const uint16_t*>(&binaryData.GetData()[576]);
	auto indices = reinterpret_cast<const uint16_t*>(&geom[model.meshParts[0].indexOffset]);
	for (uint32_t idx = 0; idx != model.meshParts[0].count; ++idx)
	{
		CHECK(indices[idx] == originalIndices[idx]);
	}
}

TEST_CASE("ModelLoader.GlbModelIndexedInterleaved")
{
	Allocator* allocator = Allocator::GetDefault();
	Filesystem filesystem(allocator, nullptr);
	Array<uint8_t> buffer(allocator);
	CHECK(filesystem.ReadBinary("test/res/model/BoxInterleaved.glb", buffer) == true);

	ModelLoader modelLoader(allocator);
	ModelData model;
	Array<uint8_t> geometryBuffer(allocator);
	CHECK(modelLoader.LoadGlbFromBuffer(&model, &geometryBuffer, buffer.GetView()) == true);

	CHECK(model.attributeCount == 2);

	CHECK(model.nodeCount == 2);

	CHECK(model.meshCount == 1);
	CHECK(model.meshes[0].indexType == RenderIndexType::UnsignedShort);
	CHECK(model.meshes[0].partCount == 1);
	CHECK(model.meshes[0].partOffset == 0);
	CHECK(model.meshes[0].primitiveMode == RenderPrimitiveMode::Triangles);
	CHECK(strcmp(model.meshes[0].name, "Mesh") == 0);

	CHECK(model.meshPartCount == 1);
	CHECK(model.meshParts[0].count == 36);
	CHECK(model.meshParts[0].uniqueVertexCount == 24);
	CHECK(model.meshParts[0].vertexFormat.attributeCount == 2);

	CHECK(model.meshParts[0].vertexFormat.attributes[0].elemCount == 3);
	CHECK(model.meshParts[0].vertexFormat.attributes[0].elemType == RenderVertexElemType::Float);
	CHECK(model.meshParts[0].vertexFormat.attributes[0].stride == 24);

	CHECK(model.meshParts[0].vertexFormat.attributes[1].elemCount == 3);
	CHECK(model.meshParts[0].vertexFormat.attributes[1].elemType == RenderVertexElemType::Float);
	CHECK(model.meshParts[0].vertexFormat.attributes[0].stride == 24);

	size_t vertexDataSize = 3 * sizeof(float) * 2 * model.meshParts[0].uniqueVertexCount;
	size_t indexDataSize = 2 * model.meshParts[0].count;
	CHECK(geometryBuffer.GetCount() >= vertexDataSize + indexDataSize);

	ArrayView<const uint8_t> binaryData = buffer.GetSubView(984, buffer.GetCount());

	// These attributes represent the data in the original glb buffer
	VertexAttribute vertexAttributes[] = { VertexAttribute::nor, VertexAttribute::pos3 };
	vertexAttributes[0].stride = 24;
	vertexAttributes[0].offset = 0;
	vertexAttributes[1].stride = 24;
	vertexAttributes[1].offset = 12;

	const uint8_t* origGeom = binaryData.GetData();
	const uint8_t* geom = geometryBuffer.GetData();
	for (uint32_t vIdx = 0; vIdx != model.meshParts[0].uniqueVertexCount; ++vIdx)
	{
		for (uint32_t aIdx = 0; aIdx != model.meshParts[0].vertexFormat.attributeCount; ++aIdx)
		{
			auto& origAttr = vertexAttributes[aIdx];
			auto& attr = model.meshParts[0].vertexFormat.attributes[aIdx];
			auto origData = reinterpret_cast<const float*>(&origGeom[origAttr.offset + origAttr.stride * vIdx]);
			auto attrData = reinterpret_cast<const float*>(&geom[attr.offset + attr.stride * vIdx]);
			for (uint32_t eIdx = 0; eIdx != attr.elemCount; ++eIdx)
			{
				CHECK(origData[eIdx] == attrData[eIdx]);
			}
		}
	}

	auto originalIndices = reinterpret_cast<const uint16_t*>(&binaryData.GetData()[576]);
	auto indices = reinterpret_cast<const uint16_t*>(&geom[model.meshParts[0].indexOffset]);
	for (uint32_t idx = 0; idx != model.meshParts[0].count; ++idx)
	{
		CHECK(indices[idx] == originalIndices[idx]);
	}
}

} // namespace kokko
