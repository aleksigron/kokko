#pragma once

#include "Core/StringView.hpp"

class Allocator;
class MaterialManager;
class TextureManager;

struct MaterialId;

namespace kokko
{

class ShaderManager;
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

	bool DeserializeMaterial(MaterialId id, ConstStringView config);
	void SerializeToString(MaterialId id, String& out);

private:
	Allocator* allocator;
	MaterialManager* materialManager;
	ShaderManager* shaderManager;
	TextureManager* textureManager;
};

}
