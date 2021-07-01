#pragma once

#include "Editor/EntityListView.hpp"
#include "Editor/EntityView.hpp"
#include "Editor/FilePickerDialog.hpp"
#include "Editor/SceneView.hpp"
#include "Editor/SelectionContext.hpp"

struct EditorViews
{
	SelectionContext selectionContext;

	EntityListView entityListView;
	EntityView entityView;
	FilePickerDialog filePicker;
	SceneView sceneView;
};
