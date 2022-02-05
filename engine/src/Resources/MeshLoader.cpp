#include "Resources/MeshLoader.hpp"

#include "Core/Core.hpp"

#include "Rendering/RenderDeviceEnums.hpp"
#include "Rendering/VertexFormat.hpp"

#include "Resources/MeshManager.hpp"

MeshLoader::MeshLoader(MeshManager* meshManager) :
	meshManager(meshManager)
{
}

MeshLoader::Status MeshLoader::LoadFromBuffer(MeshId meshId, ArrayView<uint8_t> buffer)
{
	KOKKO_PROFILE_FUNCTION();

	const uint32_t versionMajorMask = 0x00ff0000;
	const uint32_t versionMinorMask = 0x0000ff00;
	const uint32_t versionPatchMask = 0x000000ff;
	const uint32_t versionMajorShift = 16;
	const uint32_t versionMinorShift = 8;
	const uint32_t versionPatchShift = 0;
	const uint32_t versionMajor = 1;
	const uint32_t versionMinor = 0;
	const uint32_t versionPatch = 0;

	const size_t headerSize = 8 * sizeof(uint32_t);
	const size_t boundsSize = 6 * sizeof(float);

	if (buffer.GetData() == nullptr || buffer.GetCount() < headerSize)
		return Status::NoData;

	uint8_t* d = buffer.GetData();
	uint32_t* headerData = reinterpret_cast<uint32_t*>(d);

	if (headerData[0] != 0x10101991)
		return Status::FileMagicDoesNotMatch;

	// Get header data

	uint32_t versionInfo = headerData[1];

	// Make sure the file version is compatible with our loader
	if (((versionInfo & versionMajorMask) >> versionMajorShift) != versionMajor)
		return Status::FileVersionIncompatible;

	uint32_t attributeInfo = headerData[2];
	uint32_t boundsOffset = headerData[3];
	uint32_t vertexOffset = headerData[4];
	uint32_t vertexCount = headerData[5];
	uint32_t indexOffset = headerData[6];
	uint32_t indexCount = headerData[7];

	// Get vertex data components count and size

	size_t vertexSize = 0;

	uint32_t indexSize = 1 << ((attributeInfo & 0b11) - 1);
	uint32_t positionComponents = ((attributeInfo & (0b11 << 2)) >> 2) + 1;
	uint32_t normalCount = (attributeInfo & (0b1 << 4)) >> 4;
	uint32_t tangentCount = (attributeInfo & (0b1 << 5)) >> 5;
	uint32_t bitangentCount = (attributeInfo & (0b1 << 6)) >> 6;
	uint32_t colorCount = ((attributeInfo & (0b11 << 7)) >> 7);
	uint32_t texCoordCount = ((attributeInfo & (0b11 << 13)) >> 13);

	vertexSize += positionComponents * sizeof(float);
	vertexSize += normalCount * 3 * sizeof(float);
	vertexSize += tangentCount * 3 * sizeof(float);
	vertexSize += bitangentCount * 3 * sizeof(float);

	static const uint32_t MaxAttributeCount = 10;
	VertexAttribute attributes[MaxAttributeCount];
	unsigned int attributeCount = 0;

	attributes[attributeCount++] = VertexAttribute::GetPositionAttribute(positionComponents);

	if (normalCount == 1)
		attributes[attributeCount++] = VertexAttribute::nor;
	if (tangentCount == 1)
		attributes[attributeCount++] = VertexAttribute::tan;
	if (bitangentCount == 1)
		attributes[attributeCount++] = VertexAttribute::bit;

	for (uint32_t i = 0; i < colorCount; ++i)
	{
		uint32_t shift = 8 + i;
		uint32_t componentCount = ((attributeInfo & (0b1 << shift)) >> shift) + 3;
		vertexSize += componentCount * sizeof(float);
		attributes[attributeCount++] = VertexAttribute::GetColorAttribute(i, componentCount);
	}

	for (uint32_t i = 0; i < texCoordCount; ++i)
	{
		uint32_t shift = 14 + i;
		uint32_t componentCount = ((attributeInfo & (0b1 << shift)) >> shift) + 2;
		vertexSize += componentCount * sizeof(float);
		attributes[attributeCount++] = VertexAttribute::GetTextureCoordAttribute(i, componentCount);
	}

	VertexFormat format(attributes, attributeCount);
	format.CalcOffsetsAndSizeInterleaved();

	size_t vertexDataSize = vertexCount * vertexSize;
	size_t indexDataSize = indexCount * indexSize;

	// Check that the file is long enough to hold all required data
	if (indexOffset + indexDataSize > buffer.GetCount())
		return Status::FileSizeDoesNotMatch;

	float* boundsData = reinterpret_cast<float*>(d + boundsOffset);
	uint8_t* vertexData = d + vertexOffset;
	uint8_t* indexData = d + indexOffset;

	BoundingBox bounds;

	bounds.center.x = boundsData[0];
	bounds.center.y = boundsData[1];
	bounds.center.z = boundsData[2];

	bounds.extents.x = boundsData[3];
	bounds.extents.y = boundsData[4];
	bounds.extents.z = boundsData[5];

	meshManager->SetBoundingBox(meshId, bounds);

	IndexedVertexData data;
	data.vertexFormat = format;
	data.primitiveMode = RenderPrimitiveMode::Triangles;
	data.vertexData = vertexData;
	data.vertexDataSize = vertexDataSize;
	data.indexData = indexData;
	data.indexDataSize = indexDataSize;
	data.indexCount = indexCount;

	meshManager->UploadIndexed(meshId, data);

	return Status::Success;
}
