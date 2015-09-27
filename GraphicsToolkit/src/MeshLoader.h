#pragma once

struct Mesh;

namespace MeshLoader
{
	bool LoadMesh(const char* filePath, Mesh& mesh);
}
