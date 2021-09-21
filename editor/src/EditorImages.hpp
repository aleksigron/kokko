#pragma once

#include "Resources/TextureId.hpp"

class TextureManager;

class EditorImages
{
public:
	EditorImages();

	void LoadImages(TextureManager* textureManager);

	void* GetImGuiTextureId(TextureId id) const;

	TextureId folderIcon;
	TextureId genericFileIcon;

private:
	TextureManager* textureManager;
};
