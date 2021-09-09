#pragma once

#include "Editor/EditorCamera.hpp"

#include "Math/Vec2.hpp"

#include "Rendering/Framebuffer.hpp"

class InputManager;
class World;

struct CameraParameters;
struct EditorWindowInfo;
struct SelectionContext;

class SceneView
{
public:
	SceneView();

	void Initialize(RenderDevice* renderDevice, InputManager* inputManager);

	void Update();
	void Draw(EditorWindowInfo& windowInfo, World* world, SelectionContext& selectionContext);

	void ResizeFramebufferIfRequested();

	const Framebuffer& GetFramebuffer();
	Vec2i GetContentAreaSize();

	CameraParameters GetCameraParameters() const;

private:
	void ResizeFramebuffer();

	void UpdateGizmo(World* world, SelectionContext& selectionContext);

	int contentWidth;
	int contentHeight;
	int contentRegionLeft;
	int contentRegionTop;

	bool resizeRequested;
	bool windowIsFocused;
	bool windowIsHovered;

	EditorCamera editorCamera;
	Framebuffer framebuffer;
};
