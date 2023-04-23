#include "Graphics/TerrainInstance.hpp"

TerrainInstance::TerrainInstance() :
	meshId(kokko::MeshId::Null),
	terrainSize(128.0f),
	terrainResolution(128),
	textureScale(0.25f, 0.25f),
	minHeight(-0.25f),
	maxHeight(0.05f),
	heightData(nullptr),
	vertexArrayId(0),
	uniformBufferId(0),
	textureId(0)
{
}
