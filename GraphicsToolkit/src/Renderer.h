#pragma once

#include <forward_list>

#include "RenderObject.h"

class Window;
class Camera;

class Renderer
{
private:
	Window* targetWindow = nullptr;
	Camera* activeCamera = nullptr;
	
	std::forward_list<RenderObject> renderObjects;
	
public:
	Renderer();
	~Renderer();
	
	void Render();
	
	void AttachTarget(Window* window);
	void SetActiveCamera(Camera* camera);
	
	RenderObject& CreateRenderObject();
};