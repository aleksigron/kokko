#pragma once

#include "Resources/MeshData.hpp"

class MeshManager;

namespace MeshPresets
{
	void UploadCube(MeshManager* meshManager, MeshId meshId);
	void UploadPlane(MeshManager* meshManager, MeshId meshId);
}
