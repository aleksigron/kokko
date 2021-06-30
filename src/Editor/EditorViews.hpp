#pragma once

#include "Editor/EntityView.hpp"
#include "Editor/FilePickerDialog.hpp"
#include "Editor/SceneView.hpp"

struct EditorViews
{
	EntityView entityView;
	FilePickerDialog filePicker;
	SceneView sceneView;
};
