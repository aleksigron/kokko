#include "Graphics/TerrainManager.hpp"

#include "Core/Core.hpp"

#include "Graphics/TerrainInstance.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/RenderCommandList.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderViewport.hpp"
#include "Rendering/StaticUniformBuffer.hpp"

#include "Resources/MaterialManager.hpp"

TerrainManager::TerrainManager(
	Allocator* allocator,
	RenderDevice* renderDevice,
	MeshManager* meshManager,
	MaterialManager* materialManager,
	ShaderManager* shaderManager) :
	allocator(allocator),
	renderDevice(renderDevice),
	meshManager(meshManager),
	materialManager(materialManager),
	shaderManager(shaderManager),
	terrainInstances(nullptr)
{
	terrainMaterial = MaterialId{ 0 };
}

TerrainManager::~TerrainManager()
{
	allocator->MakeDelete(terrainInstances);
}

void TerrainManager::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	StringRef path("res/materials/deferred_geometry/terrain.material.json");
	terrainMaterial = materialManager->GetIdByPath(path);

	terrainInstances = allocator->MakeNew<TerrainInstance>(allocator, renderDevice, meshManager, shaderManager);
	terrainInstances->Initialize();
}

void TerrainManager::RegisterCustomRenderer(Renderer* renderer)
{
	renderer->AddCustomRenderer(this);
}

void TerrainManager::AddRenderCommands(const CustomRenderer::CommandParams& params)
{
	params.commandList->AddDrawWithCallback(params.fullscreenViewport, RenderPass::OpaqueGeometry, 0.0f, params.callbackId);
}

void TerrainManager::RenderCustom(const CustomRenderer::RenderParams& params)
{
	KOKKO_PROFILE_FUNCTION();

	const MaterialData& material = materialManager->GetMaterialData(terrainMaterial);

	terrainInstances->RenderTerrain(material, *params.viewport);
}
