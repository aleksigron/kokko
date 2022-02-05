#pragma once

class MeshManager;
class ShaderManager;
class MaterialManager;
class TextureManager;

namespace kokko
{

class ModelManager;

struct ResourceManagers
{
	MeshManager* meshManager;
	ModelManager* modelManager;
	ShaderManager* shaderManager;
	MaterialManager* materialManager;
	TextureManager* textureManager;
};

}
