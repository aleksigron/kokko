#pragma once

#include <cstddef>
#include <climits>

#include "RenderObject.h"
#include "Buffer.h"
#include "Collection.h"

class Window;
struct Camera;
struct BoundingBox;

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

	BoundingBox* boundingBoxes = nullptr;
	unsigned char* bboxCullingState = nullptr;
	
	static const size_t initialAllocation = 4096;

	inline bool RenderObjectIsAlive(unsigned int index)
	{
		return objects[index].id.innerId != UINT_MAX;
	}
	
public:
	Renderer();
	~Renderer();
	
	void Initialize();
	
	void Render();
	
	void AttachTarget(Window* window);
	void SetActiveCamera(Camera* camera);

	inline bool HasRenderObject(ObjectId id)
	{
		return objects[id.index].id.innerId == id.innerId;
	}

	inline RenderObject& GetRenderObject(ObjectId id)
	{
		return objects[id.index];
	}
	
	ObjectId AddRenderObject();
	void RemoveRenderObject(ObjectId id);
	
};