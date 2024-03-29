#pragma once

#include "Math/Vec2.hpp"

#include "Rendering/Framebuffer.hpp"

#include "App/EditorCamera.hpp"

#include "Views/EditorWindow.hpp"

class Window;

struct CameraParameters;
struct EditorWindowInfo;
struct EditorContext;

namespace kokko
{

class World;

namespace editor
{

class SceneView : public EditorWindow
{
public:
	SceneView();

	void Initialize(kokko::render::Device* renderDevice, Window* window);

	virtual void Update(EditorContext&) override;
	virtual void LateUpdate(EditorContext& context) override;

	virtual void ReleaseEngineResources() override;

	void ResizeFramebufferIfRequested();

	const kokko::render::Framebuffer& GetFramebuffer();
	Vec2i GetContentAreaSize();

	CameraParameters GetCameraParameters() const;

private:
	void ResizeFramebuffer();

	int contentWidth;
	int contentHeight;

	int currentGizmoOperation;
	int currentGizmoMode;

	bool resizeRequested;
	bool windowIsFocused;
	bool windowIsHovered;

	EditorCamera editorCamera;
	kokko::render::Framebuffer framebuffer;
};

}
}
