#pragma once

#include "Core/String.hpp"

#include "EditorWindow.hpp"

class Allocator;
class MaterialManager;
class TextureManager;


namespace kokko
{
class UniformData;

struct BufferUniform;

namespace editor
{

struct AssetInfo;

class AssetView : public EditorWindow
{
public:
	AssetView(Allocator* allocator);

	void Initialize(MaterialManager* materialManager, TextureManager* textureManager);

	virtual void Update(EditorContext& context) override;

private:
	MaterialManager* materialManager;
	TextureManager* textureManager;

	String textStore;

	void DrawMaterial(const AssetInfo* asset);
	bool DrawMaterialProperty(UniformData& uniforms, const BufferUniform& prop);
};

}
}
