#include "MeshLoader.hpp"

#include <cassert>

#include "File.hpp"
#include "Mesh.hpp"

bool MeshLoader::LoadMesh(const char* filePath, Mesh& mesh)
{
	using uint = unsigned int;
	using ushort = unsigned short;
	using ubyte = unsigned char;

	Buffer<ubyte> file = File::ReadBinary(filePath);
	size_t fileSize = file.Count();

	const uint headerSize = 16;
	const uint boundsSize = 6 * sizeof(float);

	if (file.IsValid() && fileSize >= headerSize)
	{
		ubyte* d = file.Data();
		uint* headerData = reinterpret_cast<uint*>(d);
		uint fileMagic = headerData[0];

		if (fileMagic == 0x91191010)
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

			assert(expectedSize == fileSize);

			// Check that the file size matches the header description
			if (expectedSize == fileSize)
			{
				float* boundsData = reinterpret_cast<float*>(d + headerSize);
				ubyte* vertData = d + headerSize + boundsSize;
				ushort* indexData = reinterpret_cast<ushort*>(d + headerSize + boundsSize + vertexDataSize);

				mesh.bounds.center.x = boundsData[0];
				mesh.bounds.center.y = boundsData[1];
				mesh.bounds.center.z = boundsData[2];

				mesh.bounds.extents.x = boundsData[3];
				mesh.bounds.extents.y = boundsData[4];
				mesh.bounds.extents.z = boundsData[5];

				mesh.SetPrimitiveMode(Mesh::PrimitiveMode::Triangles);

				if (normCount == 0 && colCount == 0 && texCount == 1)
				{
					BufferRef<unsigned short> indices(indexData, indexCount);
					BufferRef<Vertex3f2f> vertices;
					vertices.data = reinterpret_cast<Vertex3f2f*>(vertData);
					vertices.count = vertCount;

					mesh.Upload_3f2f(vertices, indices);
				}
				else if ((normCount == 1 && colCount == 0 && texCount == 0) ||
					(normCount == 0 && colCount == 1 && texCount == 0))
				{
					BufferRef<unsigned short> indices(indexData, indexCount);
					BufferRef<Vertex3f3f> vertices;
					vertices.data = reinterpret_cast<Vertex3f3f*>(vertData);
					vertices.count = vertCount;

					mesh.Upload_3f3f(vertices, indices);
				}
				else if (normCount == 1 && colCount == 0 && texCount == 1)
				{
					BufferRef<unsigned short> indices(indexData, indexCount);
					BufferRef<Vertex3f3f2f> vertices;
					vertices.data = reinterpret_cast<Vertex3f3f2f*>(vertData);
					vertices.count = vertCount;

					mesh.Upload_3f3f2f(vertices, indices);
				}
				else if (normCount == 1 && colCount == 1 && texCount == 0)
				{
					BufferRef<unsigned short> indices(indexData, indexCount);
					BufferRef<Vertex3f3f3f> vertices;
					vertices.data = reinterpret_cast<Vertex3f3f3f*>(vertData);
					vertices.count = vertCount;

					mesh.Upload_3f3f3f(vertices, indices);
				}

				return true;
			}
		}
	}

	return false;
}
