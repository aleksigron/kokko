#pragma once

#include "EditorCamera.hpp"

#include "Math/Vec2.hpp"

#include "Rendering/Framebuffer.hpp"

#include "EditorWindow.hpp"

class Window;
class World;

struct CameraParameters;
struct EditorWindowInfo;
struct EditorContext;

class SceneView : public EditorWindow
{
public:
	SceneView();

	void Initialize(RenderDevice* renderDevice, Window* window);

	virtual void Update(EditorContext&) override;
	virtual void LateUpdate(EditorContext& context) override;

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
