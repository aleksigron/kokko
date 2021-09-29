#pragma once

struct EditorContext;

enum class EditorWindowType
{
	Entities,
	Properties,
	Scene,
	AssetBrowser,
	Debug,
};

class EditorWindow
{
public:
	explicit EditorWindow(const char* title) :
		windowTitle(title),
		windowIsOpen(true),
		requestFocus(false)
	{
	}

	virtual ~EditorWindow()
	{
	}

	// Called after frame update
	virtual void Update(EditorContext& context)
	{
	}
	
	// Called after frame render
	virtual void LateUpdate(EditorContext& context)
	{
	}

	const char* windowTitle;
	bool windowIsOpen;
	bool requestFocus;
};
