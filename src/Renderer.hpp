#pragma once

#include "RenderPipeline.hpp"
#include "RenderOrder.hpp"
#include "RenderData.hpp"

#include "Array.hpp"

struct BoundingBox;
class Camera;
class Window;
class World;
class Scene;

class Renderer
{
private:
	unsigned int* indexList;
	unsigned int freeListFirst;

	RenderObject* objects;
	unsigned int objectCount;
	unsigned int allocatedCount;

	BoundingBox* boundingBoxes;
	unsigned char* cullingState;

	Array<RenderCommand> commands;

	RenderPipeline pipeline;

	void Reallocate();

	void UpdateBoundingBoxes(Scene* scene);
	void CreateDrawCalls(Scene* scene);
	
public:
	Renderer();
	~Renderer();

	// This function is run before calculating the world transforms of scene objects
	void PreTransformUpdate(Scene* scene);

	// Render the specified scene to the active OpenGL context
	void Render(Scene* scene);

	// Get a reference to a render object by ID
	RenderObject& GetRenderObject(unsigned int id) { return objects[indexList[id]]; }

	// Get a reference to a render object by ID
	const RenderObject& GetRenderObject(unsigned int id) const { return objects[indexList[id]]; }
	
	// Create a render object and return the created object's ID
	unsigned int AddRenderObject();

	// Remove a render object by its ID
	void RemoveRenderObject(unsigned int id);
};
