#pragma once

#include "Editor/EditorCamera.hpp"

#include "Math/Vec2.hpp"

#include "Rendering/Framebuffer.hpp"

class InputManager;

struct CameraParameters;

class SceneView
{
public:
	SceneView();

	void Initialize(RenderDevice* renderDevice, InputManager* inputManager);

	void Update();
	void Draw();

	void ResizeFramebufferIfRequested();

	const Framebuffer& GetFramebuffer();
	Vec2i GetContentAreaSize();

	CameraParameters GetCameraParameters() const;

private:
	void ResizeFramebuffer();

	int contentWidth;
	int contentHeight;

	bool resizeRequested;
	bool windowIsFocused;
	bool windowIsHovered;
	bool windowIsOpen;

	EditorCamera editorCamera;
	Framebuffer framebuffer;
};
