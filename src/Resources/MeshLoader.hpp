#pragma once

#include "Rendering/RenderDeviceEnums.hpp"
#include "Rendering/VertexFormat.hpp"

#include "Resources/MeshManager.hpp"


class MeshLoader
{
private:
	MeshManager* meshManager;

public:
	enum class Status
	{
		Success,
		NoData,
		FileMagicDoesNotMatch,
		FileVersionIncompatible,
		FileSizeDoesNotMatch,
	};

	MeshLoader(MeshManager* meshManager):
		meshManager(meshManager)
	{
	}

	Status LoadFromBuffer(MeshId meshId, BufferRef<unsigned char> buffer)
	{
		using uint = unsigned int;
		using ushort = unsigned short;
		using ubyte = unsigned char;

		const uint versionMajorMask = 0x00ff0000;
		const uint versionMinorMask = 0x0000ff00;
		const uint versionPatchMask = 0x000000ff;
		const uint versionMajorShift = 16;
		const uint versionMinorShift = 8;
		const uint versionPatchShift = 0;
		const uint versionMajor = 1;
		const uint versionMinor = 0;
		const uint versionPatch = 0;

		const uint headerSize = 8 * sizeof(uint);
		const uint boundsSize = 6 * sizeof(float);

		if (buffer.IsValid() == false || buffer.count < headerSize)
			return Status::NoData;

		ubyte* d = buffer.data;
		uint* headerData = reinterpret_cast<uint*>(d);

		if (headerData[0] != 0x10101991)
			return Status::FileMagicDoesNotMatch;

		// Get header data

		uint versionInfo = headerData[1];

		// Make sure the file version is compatible with our loader
		if (((versionInfo & versionMajorMask) >> versionMajorShift) != versionMajor)
			return Status::FileVersionIncompatible;

		uint attributeInfo = headerData[2];
		uint boundsOffset = headerData[3];
		uint vertexOffset = headerData[4];
		uint vertexCount = headerData[5];
		uint indexOffset = headerData[6];
		uint indexCount = headerData[7];

		// Get vertex data components count and size

		uint vertexSize = 0;

		uint indexSize = 1 << ((attributeInfo & 0b11) - 1);
		uint positionComponents = ((attributeInfo & (0b11 << 2)) >> 2) + 1;
		uint normalCount = (attributeInfo & (0b1 << 4)) >> 4;
		uint tangentCount = (attributeInfo & (0b1 << 5)) >> 5;
		uint bitangentCount = (attributeInfo & (0b1 << 6)) >> 6;
		uint colorCount = ((attributeInfo & (0b11 << 7)) >> 7);
		uint texCoordCount = ((attributeInfo & (0b11 << 13)) >> 13);

		vertexSize += positionComponents * sizeof(float);
		vertexSize += normalCount * 3 * sizeof(float);
		vertexSize += tangentCount * 3 * sizeof(float);
		vertexSize += bitangentCount * 3 * sizeof(float);

		static const uint MaxAttributeCount = 10;
		VertexAttribute attributes[MaxAttributeCount];
		unsigned int attributeCount = 0;

		attributes[attributeCount++] = VertexAttribute::GetPositionAttribute(positionComponents);

		if (normalCount == 1)
			attributes[attributeCount++] = VertexAttribute::nor;
		if (tangentCount == 1)
			attributes[attributeCount++] = VertexAttribute::tan;
		if (bitangentCount == 1)
			attributes[attributeCount++] = VertexAttribute::bit;
		
		for (uint i = 0; i < colorCount; ++i)
		{
			uint shift = 8 + i;
			uint componentCount = ((attributeInfo & (0b1 << shift)) >> shift) + 3;
			vertexSize += componentCount * sizeof(float);
			attributes[attributeCount++] = VertexAttribute::GetColorAttribute(i, componentCount);
		}

		for (uint i = 0; i < texCoordCount; ++i)
		{
			uint shift = 14 + i;
			uint componentCount = ((attributeInfo & (0b1 << shift)) >> shift) + 2;
			vertexSize += componentCount * sizeof(float);
			attributes[attributeCount++] = VertexAttribute::GetTextureCoordAttribute(i, componentCount);
		}

		VertexFormat format(attributes, attributeCount);

		uint vertexDataSize = vertexCount * vertexSize;
		uint indexDataSize = indexCount * indexSize;

		// Check that the file is long enough to hold all required data
		if (indexOffset + indexDataSize > buffer.count)
			return Status::FileSizeDoesNotMatch;

		float* boundsData = reinterpret_cast<float*>(d + boundsOffset);
		ubyte* vertexData = d + vertexOffset;
		ubyte* indexData = d + indexOffset;

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
		data.usage = RenderBufferUsage::StaticDraw;
		data.primitiveMode = RenderPrimitiveMode::Triangles;
		data.vertexData = vertexData;
		data.vertexCount = vertexCount;
		data.indexData = indexData;
		data.indexCount = indexCount;

		meshManager->UploadIndexed(meshId, data);

		return Status::Success;
	}
};
