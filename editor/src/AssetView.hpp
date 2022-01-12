#pragma once

#include "Core/String.hpp"

#include "EditorWindow.hpp"

class Allocator;
class MaterialManager;
class ShaderManager;
class TextureManager;
struct MaterialId;

namespace kokko
{
class UniformData;

struct BufferUniform;
struct TextureUniform;

namespace editor
{

class AssetInfo;

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

	String textStore;

	void DrawMaterial(EditorContext& context, const AssetInfo* asset);
	bool DrawMaterialProperty(UniformData& uniforms, const BufferUniform& prop);
	bool DrawMaterialShaderDropTarget(EditorContext& context, MaterialId materialId);
	bool DrawMaterialTextureDropTarget(
		EditorContext& context,
		UniformData& uniforms,
		TextureUniform& texture);

	void DrawTexture(EditorContext& context, const AssetInfo* asset);
};

}
}
