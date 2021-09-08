#pragma once

class Allocator;

#include "Core/String.hpp"

#include "Editor/DebugView.hpp"
#include "Editor/EntityListView.hpp"
#include "Editor/EntityView.hpp"
#include "Editor/FilePickerDialog.hpp"
#include "Editor/SceneView.hpp"
#include "Editor/SelectionContext.hpp"

class EditorCore
{
public:
	EditorCore(Allocator* allocator);

	void CopyEntity(World* world);
	void PasteEntity(World* world);

	SelectionContext selectionContext;

	FilePickerDialog filePicker;

	EntityListView entityListView;
	EntityView entityView;
	SceneView sceneView;
	DebugView debugView;

private:
	String copiedEntity;
};
