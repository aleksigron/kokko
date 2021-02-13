#pragma once

class Allocator;
class RenderDevice;
class MeshManager;

#include "Core/Array.hpp"

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"

#include "Resources/MeshData.hpp"

struct MaterialData;

class TerrainInstance
{
public:
	using HeightType = unsigned short;

private:
	Allocator* allocator;
	RenderDevice* renderDevice;
	MeshManager* meshManager;
	MeshId meshId;

	Array<HeightType> heightValues;
	float terrainSize;
	int terrainResolution;

	unsigned int vertexArray;

	void InitializeVertexArray();

public:
	TerrainInstance(Allocator* allocator, RenderDevice* renderDevice, MeshManager* meshManager);
	~TerrainInstance();

	void RenderTerrain(const MaterialData& material);
};
