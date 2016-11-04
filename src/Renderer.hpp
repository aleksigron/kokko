#pragma once

#include "RenderObject.hpp"
#include "RenderOrder.hpp"
#include "Array.hpp"

struct BoundingBox;
class Camera;
class Window;
class World;
class Scene;

class Renderer
{
private:
	Window* targetWindow;
	Camera* activeCamera;

	unsigned int* indexList;
	unsigned int freeListFirst;

	RenderObject* objects;
	unsigned int objectCount;
	unsigned int allocatedCount;

	BoundingBox* boundingBoxes;
	unsigned char* cullingState;

	Array<DrawCall> drawCalls;

	RenderOrderConfiguration renderOrderConfiguration;

	void Reallocate();

	void InitializeRenderOrder();

	void UpdateBoundingBoxes(Scene* scene);
	void CreateDrawCalls(Scene* scene);
	
public:
	Renderer();
	~Renderer();

	void PreTransformUpdate();
	void Render(Scene* scene);
	
	void AttachTarget(Window* window);
	void SetActiveCamera(Camera* camera);

	RenderObject& GetRenderObject(unsigned int id) { return objects[indexList[id]]; }
	const RenderObject& GetRenderObject(unsigned int id) const { return objects[indexList[id]]; }
	
	unsigned int AddRenderObject();
	void RemoveRenderObject(unsigned int id);
};
