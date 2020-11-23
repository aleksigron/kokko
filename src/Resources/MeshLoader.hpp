#pragma once

#include "Rendering/RenderDeviceEnums.hpp"

#include "Resources/MeshManager.hpp"

class MeshLoader
{
private:
	MeshManager* meshManager;
	MeshId meshId;

public:
	MeshLoader(MeshManager* meshManager, MeshId meshId):
		meshManager(meshManager),
		meshId(meshId)
	{
	}

	bool LoadFromBuffer(BufferRef<unsigned char> buffer)
	{
		using uint = unsigned int;
		using ushort = unsigned short;
		using ubyte = unsigned char;

		const uint headerSize = 16;
		const uint boundsSize = 6 * sizeof(float);

		if (buffer.IsValid() && buffer.count >= headerSize)
		{
			ubyte* d = buffer.data;
			uint* headerData = reinterpret_cast<uint*>(d);

			if (headerData[0] == 0x91191010)
			{
				// Get header data

				uint vertComps = headerData[1];
				uint vertCount = headerData[2];
				uint indexCount = headerData[3];

				// Get vertex data components count and size

				uint posCount = (vertComps & 0x01) >> 0;
				uint posSize = posCount * 3 * sizeof(float);

				uint normCount = (vertComps & 0x02) >> 1;
				uint normSize = normCount * 3 * sizeof(float);

				uint colCount = (vertComps & 0x04) >> 2;
				uint colSize = colCount * 3 * sizeof(float);

				uint texCount = (vertComps & 0x08) >> 3;
				uint texSize = texCount * 2 * sizeof(float);

				uint vertSize = posSize + normSize + colSize + texSize;
				uint vertexDataSize = vertCount * vertSize;

				uint indexSize = indexCount > (1 << 16) ? 4 : 2;
				uint indexDataSize = indexCount * indexSize;

				uint expectedSize = headerSize + boundsSize + vertexDataSize + indexDataSize;

				assert(expectedSize == buffer.count);

				// Check that the file size matches the header description
				if (expectedSize == buffer.count)
				{
					float* boundsData = reinterpret_cast<float*>(d + headerSize);
					ubyte* vertData = d + headerSize + boundsSize;
					ushort* indexData = reinterpret_cast<ushort*>(d + headerSize + boundsSize + vertexDataSize);

					BoundingBox bounds;

					bounds.center.x = boundsData[0];
					bounds.center.y = boundsData[1];
					bounds.center.z = boundsData[2];

					bounds.extents.x = boundsData[3];
					bounds.extents.y = boundsData[4];
					bounds.extents.z = boundsData[5];

					meshManager->SetBoundingBox(meshId, bounds);

					if (normCount == 0 && colCount == 0 && texCount == 0)
					{
						IndexedVertexData<Vertex3f, unsigned short> data;
						data.primitiveMode = RenderPrimitiveMode::Triangles;
						data.idxData = indexData;
						data.idxCount = indexCount;
						data.vertData = reinterpret_cast<Vertex3f*>(vertData);
						data.vertCount = vertCount;

						meshManager->UploadIndexed_3f(meshId, data, RenderBufferUsage::StaticDraw);

						return true;
					}
					else if (normCount == 0 && colCount == 0 && texCount == 1)
					{
						IndexedVertexData<Vertex3f2f, unsigned short> data;
						data.primitiveMode = RenderPrimitiveMode::Triangles;
						data.idxData = indexData;
						data.idxCount = indexCount;
						data.vertData = reinterpret_cast<Vertex3f2f*>(vertData);
						data.vertCount = vertCount;

						meshManager->Upload_3f2f(meshId, data, RenderBufferUsage::StaticDraw);

						return true;
					}
					else if ((normCount == 1 && colCount == 0 && texCount == 0) ||
							 (normCount == 0 && colCount == 1 && texCount == 0))
					{
						IndexedVertexData<Vertex3f3f, unsigned short> data;
						data.primitiveMode = RenderPrimitiveMode::Triangles;
						data.idxData = indexData;
						data.idxCount = indexCount;
						data.vertData = reinterpret_cast<Vertex3f3f*>(vertData);
						data.vertCount = vertCount;

						meshManager->Upload_3f3f(meshId, data, RenderBufferUsage::StaticDraw);

						return true;
					}
					else if (normCount == 1 && colCount == 0 && texCount == 1)
					{
						IndexedVertexData<Vertex3f3f2f, unsigned short> data;
						data.primitiveMode = RenderPrimitiveMode::Triangles;
						data.idxData = indexData;
						data.idxCount = indexCount;
						data.vertData = reinterpret_cast<Vertex3f3f2f*>(vertData);
						data.vertCount = vertCount;

						meshManager->Upload_3f3f2f(meshId, data, RenderBufferUsage::StaticDraw);

						return true;
					}
					else if (normCount == 1 && colCount == 1 && texCount == 0)
					{
						IndexedVertexData<Vertex3f3f3f, unsigned short> data;
						data.primitiveMode = RenderPrimitiveMode::Triangles;
						data.idxData = indexData;
						data.idxCount = indexCount;
						data.vertData = reinterpret_cast<Vertex3f3f3f*>(vertData);
						data.vertCount = vertCount;

						meshManager->Upload_3f3f3f(meshId, data, RenderBufferUsage::StaticDraw);

						return true;
					}
				}
			}
		}

		return false;
	}
};
