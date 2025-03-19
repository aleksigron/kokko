#pragma once

#include "Core/StringView.hpp"

namespace kokko
{

class Allocator;
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
