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
	objectUniformBufferId = 0;
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

	renderDevice->CreateBuffers(1, &objectUniformBufferId);
	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, objectUniformBufferId);
	renderDevice->SetBufferData(RenderBufferTarget::UniformBuffer, sizeof(TransformUniformBlock), nullptr, RenderBufferUsage::DynamicDraw);
}

void TerrainManager::AddRenderCommands(const CustomRenderer::CommandParams& params)
{
	params.commandList->AddDrawWithCallback(params.fullscreenViewport, RenderPass::OpaqueGeometry, 0.0f, params.callbackId);
}

void TerrainManager::RenderCustom(const CustomRenderer::RenderParams& params)
{
	TransformUniformBlock objectUniforms;

	const MaterialData& material = materialManager->GetMaterialData(terrainMaterial);

	objectUniforms.MVP = params.viewport->viewProjection;
	objectUniforms.MV = params.viewport->view;
	objectUniforms.M = Mat4x4f();

	renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, objectUniformBufferId);
	renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(TransformUniformBlock), &objectUniforms);

	// Bind object transform uniform block to shader
	renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, TransformUniformBlock::BindingPoint, objectUniformBufferId);

	terrainInstances->RenderTerrain(material);
}
