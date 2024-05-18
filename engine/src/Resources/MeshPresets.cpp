#include "Resources/MeshPresets.hpp"

#include "Resources/MeshId.hpp"
#include "Resources/ModelManager.hpp"

#include "Rendering/RenderTypes.hpp"
#include "Rendering/VertexFormat.hpp"

namespace kokko
{

ModelId MeshPresets::CreateCube(ModelManager* modelManager)
{
	static const float vertexData[] = {
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f
	};

	static const unsigned short indexData[] = {
		0, 5, 4, 0, 1, 5,
		4, 7, 6, 4, 5, 7,
		5, 3, 7, 5, 1, 3,
		2, 1, 0, 2, 3, 1,
		0, 6, 2, 0, 4, 6,
		3, 6, 7, 3, 2, 6
	};

	VertexAttribute vertexAttributes[] = { VertexAttribute::pos3 };
	VertexFormat vertexFormatPos(vertexAttributes, sizeof(vertexAttributes) / sizeof(vertexAttributes[0]));
	vertexFormatPos.CalcOffsetsAndSizeInterleaved();

	ModelCreateInfo info;
	info.vertexFormat = vertexFormatPos;
	info.primitiveMode = RenderPrimitiveMode::Triangles;
	info.vertexData = vertexData;
	info.vertexDataSize = sizeof(vertexData);
	info.vertexCount = sizeof(vertexData) / (sizeof(vertexData[0]) * 3);
	info.indexData = indexData;
	info.indexDataSize = sizeof(indexData);
	info.indexCount = sizeof(indexData) / sizeof(indexData[0]);
	info.indexType = RenderIndexType::UnsignedShort;

	ModelId modelId = modelManager->CreateModel(info);

	AABB bounds;
	bounds.center = Vec3f(0.0f, 0.0f, 0.0f);
	bounds.extents = Vec3f(0.5f, 0.5f, 0.5f);
	modelManager->SetMeshAABB(MeshId{modelId, 0}, bounds);

	return modelId;
}

ModelId MeshPresets::CreatePlane(ModelManager* modelManager)
{
	static const float vertexData[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};

	static const unsigned short indexData[] = { 0, 1, 2, 1, 3, 2 };

	VertexAttribute vertexAttributes[] = { VertexAttribute::pos3 };
	VertexFormat vertexFormatPos(vertexAttributes, sizeof(vertexAttributes) / sizeof(vertexAttributes[0]));
	vertexFormatPos.CalcOffsetsAndSizeInterleaved();

	ModelCreateInfo info;
	info.vertexFormat = vertexFormatPos;
	info.vertexData = vertexData;
	info.vertexDataSize = sizeof(vertexData);
	info.vertexCount = sizeof(vertexData) / (sizeof(vertexData[0]) * 3);
	info.indexData = indexData;
	info.indexDataSize = sizeof(indexData);
	info.indexCount = sizeof(indexData) / sizeof(indexData[0]);
	info.indexType = RenderIndexType::UnsignedShort;

	ModelId modelId = modelManager->CreateModel(info);

	AABB bounds;
	bounds.center = Vec3f(0.0f, 0.0f, 0.0f);
	bounds.extents = Vec3f(0.5f, 0.5f, 0.5f);
	modelManager->SetMeshAABB(MeshId{modelId, 0}, bounds);

	return modelId;
}

} // namespace kokko
