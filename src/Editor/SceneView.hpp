#pragma once

#include "Math/Vec2.hpp"

#include "Rendering/Framebuffer.hpp"

class SceneView
{
public:
	SceneView();

	void Initialize(RenderDevice* renderDevice);

	void Draw();

	const Framebuffer& GetFramebuffer();

private:
	void ResizeFramebuffer();

	int contentWidth;
	int contentHeight;

	Framebuffer framebuffer;
};
