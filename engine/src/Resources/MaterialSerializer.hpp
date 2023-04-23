#pragma once

#include "Core/StringView.hpp"

class Allocator;

namespace kokko
{

class MaterialManager;
class ShaderManager;
class String;
class TextureManager;

struct MaterialId;

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

} // namespace kokko
