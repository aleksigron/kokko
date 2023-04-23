#pragma once

#include "Resources/TextureId.hpp"

namespace kokko
{

class TextureManager;

namespace editor
{

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

}
}
