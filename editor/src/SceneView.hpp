#pragma once

#include "EditorCamera.hpp"

#include "Math/Vec2.hpp"

#include "Rendering/Framebuffer.hpp"

class Window;
class World;

struct CameraParameters;
struct EditorWindowInfo;
struct SelectionContext;

class SceneView
{
public:
	SceneView();

	void Initialize(RenderDevice* renderDevice, Window* window);

	void Update();
	void Draw(EditorWindowInfo& windowInfo, World* world, SelectionContext& selectionContext);

	void ResizeFramebufferIfRequested();

	const Framebuffer& GetFramebuffer();
	Vec2i GetContentAreaSize();

	CameraParameters GetCameraParameters() const;

private:
	void ResizeFramebuffer();

	int contentWidth;
	int contentHeight;

	int currentGizmoOperation;

	bool resizeRequested;
	bool windowIsFocused;
	bool windowIsHovered;

	EditorCamera editorCamera;
	Framebuffer framebuffer;
};
