#pragma once

#include <cstdint>

#include "Core/Array.hpp"

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"

#include "Resources/MeshData.hpp"

class Allocator;
class RenderDevice;
class MeshManager;
class ShaderManager;

struct MaterialData;

class TerrainInstance
{
public:
	using HeightType = unsigned short;

private:
	Allocator* allocator;
	RenderDevice* renderDevice;
	MeshManager* meshManager;
	ShaderManager* shaderManager;
	MeshId meshId;

	Array<HeightType> heightValues;
	float terrainSize;
	int terrainResolution;

	float minHeight;
	float maxHeight;
	uint16_t* heightData;

	unsigned int vertexArrayId;
	unsigned int textureId;

public:
	TerrainInstance(Allocator* allocator, RenderDevice* renderDevice,
		MeshManager* meshManager, ShaderManager* shaderManager);
	~TerrainInstance();

	void Initialize();

	void RenderTerrain(const MaterialData& material);
};
