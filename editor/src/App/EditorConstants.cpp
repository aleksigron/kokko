#include "EditorConstants.hpp"

namespace kokko
{
namespace editor
{
const char* const EditorConstants::EditorResourcePath = "editor/res";
const char* const EditorConstants::VirtualMountEditor = "editor";
const char* const EditorConstants::UserSettingsFilePath = "editor_user_settings.yml";
const char* const EditorConstants::AssetDirectoryName = "Assets";
const char* const EditorConstants::SceneDragDropType = "SceneObject";
const char* const EditorConstants::AssetDragDropType = "Asset";
const char* const EditorConstants::ModelMeshDragDropType = "ModelMesh";

const char* const EditorConstants::NewAssetContentMaterial = R"RAW({
	"shader": "beefdc5ec8726697d0282955735586ec",
	"variables": [
		{
			"name": "color_tint",
			"value": [ 0.8, 0.8, 0.8 ]
		},
		{
			"name": "metalness",
			"value": 0.0
		},
		{
			"name": "roughness",
			"value": 0.3
		}
	]
}
)RAW";
}
}
