#pragma once

#include "Core/String.hpp"

#include "EditorWindow.hpp"

class Allocator;

namespace kokko
{

class AssetInfo;
class MaterialManager;
class ShaderManager;
class TextureManager;
class UniformData;

struct BufferUniform;
struct MaterialId;
struct TextureUniform;

namespace editor
{

class AssetView : public EditorWindow
{
public:
	AssetView(Allocator* allocator);

	void Initialize(
		MaterialManager* materialManager,
		ShaderManager* shaderManager,
		TextureManager* textureManager);

	virtual void Update(EditorContext& context) override;

private:
	Allocator* allocator;
	MaterialManager* materialManager;
	ShaderManager* shaderManager;
	TextureManager* textureManager;

	void DrawMaterial(EditorContext& context, const AssetInfo* asset);
	bool DrawMaterialProperty(EditorContext& context, UniformData& uniforms, const BufferUniform& prop);
	bool DrawMaterialShaderDropTarget(EditorContext& context, kokko::MaterialId materialId);
	bool DrawMaterialTextureDropTarget(
		EditorContext& context,
		UniformData& uniforms,
		TextureUniform& texture);

	void DrawTexture(EditorContext& context, const AssetInfo* asset);
};

}
}
