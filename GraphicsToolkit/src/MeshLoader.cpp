#include "MeshLoader.h"

#include "File.h"
#include "Mesh.h"

bool MeshLoader::LoadMesh(const char* filePath, Mesh& mesh)
{
	Buffer<unsigned char> file = File::Read(filePath);
	size_t fileSize = file.Count();

	if (file.IsValid() && fileSize >= 6)
	{
		unsigned char* d = file.Data();
		unsigned int* fileMagic = reinterpret_cast<unsigned int*>(d);

		if (*fileMagic == 0x91191010)
		{
			unsigned int vertCount = *reinterpret_cast<unsigned short*>(d + 4);

			bool fileSizeMatches = fileSize == (6 + vertCount * 12);

			if (fileSizeMatches)
			{
				float* vertData = reinterpret_cast<float*>(d + 6);

				for (unsigned i = 0; i < vertCount; ++i)
				{
					float x = *(vertData + i * 3 + 0);
					float y = *(vertData + i * 3 + 1);
					float z = *(vertData + i * 3 + 2);
				}
			}
		}
	}

	return false;
}
