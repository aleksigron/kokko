#pragma once

namespace kokko
{

class MaterialManager;
class MeshManager;
class ModelManager;
class ShaderManager;
class TextureManager;

struct ResourceManagers
{
	MeshManager* meshManager;
	ModelManager* modelManager;
	ShaderManager* shaderManager;
	MaterialManager* materialManager;
	TextureManager* textureManager;
};

} // namespace kokko
