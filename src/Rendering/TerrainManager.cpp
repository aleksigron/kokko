#include "Rendering/TerrainManager.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/RenderCommandList.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderViewport.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/TerrainInstance.hpp"

#include "Resources/MaterialManager.hpp"

TerrainManager::TerrainManager(
	Allocator* allocator,
	RenderDevice* renderDevice,
	MeshManager* meshManager,
	MaterialManager* materialManager) :
	allocator(allocator),
	renderDevice(renderDevice),
	meshManager(meshManager),
	materialManager(materialManager),
	terrainInstances(nullptr)
{
	terrainMaterial = MaterialId{ 0 };
}

TerrainManager::~TerrainManager()
{
	allocator->MakeDelete(terrainInstances);
}

void TerrainManager::Initialize(Renderer* renderer, ShaderManager* shaderManager)
{
	renderer->AddCustomRenderer(this);
	
	StringRef path("res/materials/deferred_geometry/terrain.material.json");
	terrainMaterial = materialManager->GetIdByPath(path);

	terrainInstances = allocator->MakeNew<TerrainInstance>(allocator, renderDevice, meshManager, shaderManager);
	terrainInstances->Initialize();
}

void TerrainManager::AddRenderCommands(const CustomRenderer::CommandParams& params)
{
	params.commandList->AddDrawWithCallback(params.fullscreenViewport, RenderPass::OpaqueGeometry, 0.0f, params.callbackId);
}

void TerrainManager::RenderCustom(const CustomRenderer::RenderParams& params)
{
	const MaterialData& material = materialManager->GetMaterialData(terrainMaterial);

	terrainInstances->RenderTerrain(material, *params.viewport);
}
