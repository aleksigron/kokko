#include "EditorConstants.hpp"

namespace kokko
{
namespace editor
{
const char* const EditorConstants::MetadataExtensionStr = ".meta";
const std::filesystem::path EditorConstants::MetadataExtension = std::filesystem::path(EditorConstants::MetadataExtensionStr);

const char* const EditorConstants::AssetDirectoryName = "Assets";

const char* const EditorConstants::EngineResourcePath = "engine/res";
const char* const EditorConstants::EditorResourcePath = "editor/res";

const char* const EditorConstants::VirtualMountEngine = "engine";
const char* const EditorConstants::VirtualMountEditor = "editor";
const char* const EditorConstants::VirtualMountAssets = "assets";

const char* const EditorConstants::UserSettingsFilePath = "editor_user_settings.yml";

const char* const EditorConstants::SceneDragDropType = "SceneObject";
const char* const EditorConstants::AssetDragDropType = "Asset";
const char* const EditorConstants::MaterialDragDropType = "Material";


}
}
