#pragma once

namespace kokko
{
struct ModelId;
class ModelManager;

namespace MeshPresets
{
ModelId CreateCube(ModelManager* modelManager);
ModelId CreatePlane(ModelManager* modelManager);
}

} // namespace kokko
