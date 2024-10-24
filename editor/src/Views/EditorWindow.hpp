#pragma once

#include <cstdint>

namespace kokko
{
namespace editor
{

struct EditorContext;

enum class EditorWindowGroup : uint8_t
{
	Regular = 0,
	Debug = 1
};

class EditorWindow
{
public:
	EditorWindow(const char* title, EditorWindowGroup group) :
		windowTitle(title),
		windowGroup(group),
		windowIsOpen(true),
		requestFocus(false)
	{
	}

	virtual ~EditorWindow() {}

	virtual void OnEditorProjectChanged(const EditorContext& context) {}

	// Called after frame update
	virtual void Update(EditorContext& context) {}

	// Called after frame render
	virtual void LateUpdate(EditorContext& context) {}

	virtual void ReleaseEngineResources() {}

	const char* windowTitle;
	EditorWindowGroup windowGroup;
	bool windowIsOpen;
	bool requestFocus;
};

} // namespace editor
} // namespace kokko
