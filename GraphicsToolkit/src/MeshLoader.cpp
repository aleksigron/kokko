#include "MeshLoader.h"

#include "File.h"
#include "Mesh.h"

bool MeshLoader::LoadMesh(const char* filePath, Mesh& mesh)
{
	Buffer<unsigned char> file = File::Read(filePath);
	size_t fileSize = file.Count();

	const unsigned int headerSize = 16;

	if (file.IsValid() && fileSize >= headerSize)
	{
		unsigned char* d = file.Data();
		unsigned int* headerData = reinterpret_cast<unsigned int*>(d);
		unsigned int fileMagic = headerData[0];

		if (fileMagic == 0x91191010)
		{
			// Get header data

			unsigned vertComps = headerData[1];
			unsigned vertCount = headerData[2];
			unsigned indexCount = headerData[3];

			// Get vertex data components count and size

			unsigned posSize = ((vertComps & 0x01) >> 0) * 3 * sizeof(float);
			unsigned normSize = ((vertComps & 0x02) >> 1) * 3 * sizeof(float);
			unsigned colSize = ((vertComps & 0x04) >> 2) * 3 * sizeof(float);
			unsigned texSize = ((vertComps & 0x08) >> 3) * 2 * sizeof(float);
			unsigned vertSize = posSize + normSize + colSize + texSize;
			unsigned vertexDataSize = vertCount * vertSize;

			unsigned indexSize = indexCount > (1 << 16) ? 4 : 2;
			unsigned indexDataSize = indexCount * indexSize;

			unsigned expectedSize = headerSize + vertexDataSize + indexDataSize;

			// Check that the file size matches the header description
			if (expectedSize == fileSize)
			{
				float* vertData = reinterpret_cast<float*>(d + headerSize);
				unsigned short* indexData = reinterpret_cast<unsigned short*>(d + headerSize + vertexDataSize);

				// TODO: Get the right vertex data type
				mesh.Upload_PosCol(vertData, vertCount, indexData, indexCount);

				return true;
			}
		}
	}

	return false;
}
