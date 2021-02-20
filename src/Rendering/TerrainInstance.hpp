#pragma once

#include <cstdint>

#include "Core/Array.hpp"

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Mat4x4.hpp"

#include "Resources/MeshData.hpp"

class Allocator;
class RenderDevice;
class MeshManager;
class ShaderManager;

struct MaterialData;
struct RenderViewport;

class TerrainInstance
{
public:
	using HeightType = unsigned short;

private:
	struct UniformBlock
	{
		static const unsigned int BindingPoint = 2;

		alignas(16) Mat4x4f MVP;
		alignas(16) Mat4x4f MV;

		alignas(8) Vec2f textureScale;

		alignas(4) float terrainSize;
		alignas(4) float terrainResolution;
		alignas(4) float minHeight;
		alignas(4) float maxHeight;
	};

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
	unsigned int uniformBufferId;
	unsigned int textureId;

public:
	TerrainInstance(Allocator* allocator, RenderDevice* renderDevice,
		MeshManager* meshManager, ShaderManager* shaderManager);
	~TerrainInstance();

	void Initialize();

	void RenderTerrain(const MaterialData& material, const RenderViewport& viewport);
};
