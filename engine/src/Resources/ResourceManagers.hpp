#pragma once

class MeshManager;
class ShaderManager;
class MaterialManager;
class TextureManager;

struct ResourceManagers
{
	MeshManager* meshManager;
	ShaderManager* shaderManager;
	MaterialManager* materialManager;
	TextureManager* textureManager;
};
