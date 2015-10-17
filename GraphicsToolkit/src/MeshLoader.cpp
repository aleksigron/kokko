#include "MeshLoader.hpp"

#include "File.hpp"
#include "Mesh.hpp"

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

			unsigned posCount = (vertComps & 0x01) >> 0;
			unsigned posSize = posCount * 3 * sizeof(float);

			unsigned normCount = (vertComps & 0x02) >> 1;
			unsigned normSize = normCount * 3 * sizeof(float);

			unsigned colCount = (vertComps & 0x04) >> 2;
			unsigned colSize = colCount * 3 * sizeof(float);

			unsigned texCount = (vertComps & 0x08) >> 3;
			unsigned texSize = texCount * 2 * sizeof(float);

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

				if (normCount == 1 && colCount == 0 && texCount == 0)
					mesh.Upload_PosNor(vertData, vertCount, indexData, indexCount);

				else if (normCount == 0 && colCount == 1 && texCount == 0)
					mesh.Upload_PosCol(vertData, vertCount, indexData, indexCount);

				else if (normCount == 1 && colCount == 1 && texCount == 0)
					mesh.Upload_PosNorCol(vertData, vertCount, indexData, indexCount);

				return true;
			}
		}
	}

	return false;
}
