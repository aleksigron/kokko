#pragma once

#include <forward_list>

#include "RenderObject.h"

class Window;

class Renderer
{
private:
	Window* targetWindow = nullptr;
	
	std::forward_list<RenderObject> renderObjects;
	
public:
	Renderer();
	~Renderer();
	
	void Render();
	
	void AttachTarget(Window* window);
	
	RenderObject& CreateRenderObject();
};