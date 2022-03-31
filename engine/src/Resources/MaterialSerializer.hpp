#pragma once

#include "Core/StringRef.hpp"

class Allocator;
class MaterialManager;
class ShaderManager;
class TextureManager;

struct MaterialId;

namespace kokko
{

class String;

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
	void SerializeToString(MaterialId id, String& out);

private:
	Allocator* allocator;
	MaterialManager* materialManager;
	ShaderManager* shaderManager;
	TextureManager* textureManager;
};

}
