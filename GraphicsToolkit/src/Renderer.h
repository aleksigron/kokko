#pragma once

#include <cstddef>

#include "RenderObject.h"
#include "Buffer.h"
#include "Collection.h"

class Window;
class Camera;

class Renderer
{
private:
	Window* targetWindow = nullptr;
	Camera* activeCamera = nullptr;
	
	uint32_t nextInnerId = 0;
	uint32_t freeList = UINT32_MAX;
	
	RenderObject* objects = nullptr;
	size_t contiguousFree = 0;
	size_t allocatedCount = 0;
	
	static const size_t initialAllocation = 4096;
	
public:
	Renderer();
	~Renderer();
	
	void Initialize();
	
	void Render();
	
	void AttachTarget(Window* window);
	void SetActiveCamera(Camera* camera);
	
	bool HasRenderObject(RenderObjectId id);
	RenderObject& GetRenderObject(RenderObjectId id);
	
	RenderObjectId AddRenderObject();
	void RemoveRenderObject(RenderObjectId id);
	
};