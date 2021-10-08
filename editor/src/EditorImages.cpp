#include "EditorImages.hpp"

#include "Resources/TextureManager.hpp"

namespace kokko
{
namespace editor
{

EditorImages::EditorImages() :
	folderIcon(TextureId::Null),
	genericFileIcon(TextureId::Null),
	textureManager(nullptr)
{
}

void EditorImages::LoadImages(TextureManager* textureManager)
{
	this->textureManager = textureManager;

	folderIcon = textureManager->GetIdByPath(StringRef("editor/textures/editor_folder_icon_256.png"));
	genericFileIcon = textureManager->GetIdByPath(StringRef("editor/textures/editor_generic_file_icon_256.png"));
}

void* EditorImages::GetImGuiTextureId(TextureId id) const
{
	const TextureData& data = textureManager->GetTextureData(id);
	return reinterpret_cast<void*>(static_cast<size_t>(data.textureObjectId));
}

}
}
