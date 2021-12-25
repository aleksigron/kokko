#pragma once

class Allocator;
class MaterialManager;
class ShaderManager;
class TextureManager;

struct MaterialId;
struct StringRef;

namespace kokko
{

class MaterialSerializer
{
public:
	MaterialSerializer(
		Allocator* allocator,
		MaterialManager* materialManager,
		ShaderManager* shaderManager,
		TextureManager* textureManager);
	~MaterialSerializer();

	bool DeserializeMaterial(MaterialId id, StringRef config);

private:
	Allocator* allocator;
	MaterialManager* materialManager;
	ShaderManager* shaderManager;
	TextureManager* textureManager;
};

}
