#pragma once

class MeshManager;
class ShaderManager;
class MaterialManager;
class TextureManager;
class EnvironmentManager;

struct ResourceManagers
{
	MeshManager* meshManager;
	ShaderManager* shaderManager;
	MaterialManager* materialManager;
	TextureManager* textureManager;
	EnvironmentManager* environmentManager;
};
