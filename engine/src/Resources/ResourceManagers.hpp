#pragma once

class MeshManager;
class MaterialManager;
class TextureManager;

namespace kokko
{

class ModelManager;
class ShaderManager;

struct ResourceManagers
{
	MeshManager* meshManager;
	ModelManager* modelManager;
	ShaderManager* shaderManager;
	MaterialManager* materialManager;
	TextureManager* textureManager;
};

}
