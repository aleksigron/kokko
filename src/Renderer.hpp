#pragma once

#include <cstddef>
#include <climits>

#include "RenderObject.hpp"
#include "Buffer.hpp"

struct BoundingBox;
struct Camera;
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
	unsigned char* bboxCullingState;

	void Reallocate();
	void UpdateBoundingBoxes(Scene* scene);
	
public:
	Renderer();
	~Renderer();

	void PreTransformUpdate();
	void Render(const World* world, Scene* scene);
	
	void AttachTarget(Window* window);
	void SetActiveCamera(Camera* camera);

	RenderObject& GetRenderObject(unsigned int id) { return objects[indexList[id]]; }
	const RenderObject& GetRenderObject(unsigned int id) const { return objects[indexList[id]]; }
	
	unsigned int AddRenderObject();
	void RemoveRenderObject(unsigned int id);
};
