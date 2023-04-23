#pragma once

namespace kokko
{
struct MeshId;
class MeshManager;

namespace MeshPresets
{
void UploadCube(MeshManager* meshManager, MeshId meshId);
void UploadPlane(MeshManager* meshManager, MeshId meshId);
}

} // namespace kokko
