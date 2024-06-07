#include "Resources/ModelManager.hpp"

#include "Core/Array.hpp"
#include "Core/Core.hpp"

#include "Resources/AssetLoader.hpp"
#include "Resources/MeshId.hpp"

#include "Rendering/RenderDevice.hpp"

namespace kokko
{

ModelManager::ModelManager(Allocator* allocator, AssetLoader* assetLoader, render::Device* renderDevice) :
	allocator(allocator),
	assetLoader(assetLoader),
	renderDevice(renderDevice),
	modelLoader(allocator),
	uidMap(allocator)
{
	Reallocate(16);

	data.slotsUsed = 1; // Reserve index 0 for ModelId::Null
}

ModelManager::~ModelManager()
{
	for (size_t i = 1; i < data.slotsUsed; ++i)
		if (data.model[i].buffer != nullptr)
			allocator->Deallocate(data.model[i].buffer);
}

ModelId ModelManager::FindModelByUid(const kokko::Uid& uid)
{
	KOKKO_PROFILE_FUNCTION();

	auto* pair = uidMap.Lookup(uid);
	if (pair != nullptr)
		return ModelId{ pair->second };

	Array<uint8_t> file(allocator);

	if (assetLoader->LoadAsset(uid, file))
	{
		uint32_t id = AcquireSlot();
		ModelData& model = data.model[id];
		model.uid = uid;
		model.hasUid = true;

		Array<uint8_t> geometryBuffer(allocator);

		if (modelLoader.LoadGlbFromBuffer(&model, &geometryBuffer, file.GetView()))
		{
			CreateRenderData(model, geometryBuffer);

			pair = uidMap.Insert(uid);
			pair->second = id;

			return ModelId{ id };
		}
		else
		{
			char uidStr[Uid::StringLength + 1];
			uid.WriteTo(uidStr);
			uidStr[Uid::StringLength] = '\0';

			KK_LOG_ERROR("Model with UID {} failed to be loaded.", uidStr);
			ReleaseSlot(id);
		}
	}

	return ModelId::Null;
}

ModelId ModelManager::FindModelByPath(const ConstStringView& path)
{
	KOKKO_PROFILE_FUNCTION();

	auto uidResult = assetLoader->GetAssetUidByVirtualPath(path);
	if (uidResult.HasValue())
	{
		return FindModelByUid(uidResult.GetValue());
	}

	return ModelId::Null;
}

ModelId ModelManager::CreateModel(const ModelCreateInfo& info)
{
	KOKKO_PROFILE_FUNCTION();

	assert(info.indexType == RenderIndexType::None || (info.indexData != nullptr && info.indexDataSize != 0 && info.indexCount != 0));

	uint32_t id = AcquireSlot();
	ModelData& model = data.model[id];

	Array<uint8_t> geometryBuffer(allocator);

	if (modelLoader.LoadRuntime(&model, &geometryBuffer, info))
	{
		CreateRenderData(model, geometryBuffer);

		return ModelId{id};
	}

	ReleaseSlot(id);
    return ModelId::Null;
}

void ModelManager::RemoveModel(ModelId id)
{
	assert(id != ModelId::Null);

	ModelData& model = data.model[id.i];

	if (model.hasUid)
		if (auto mapPair = uidMap.Lookup(model.uid))
			uidMap.Remove(mapPair);

	ReleaseRenderData(model);

	allocator->Deallocate(model.buffer);
	model.buffer = nullptr;

	ReleaseSlot(id.i);
}

void ModelManager::SetMeshAABB(MeshId id, const AABB& bounds)
{
	assert(id != MeshId::Null);
	const ModelData& model = data.model[id.modelId.i];
	assert(id.meshIndex < model.meshCount);
	model.meshes[id.meshIndex].aabb = bounds;
}

Optional<Uid> ModelManager::GetModelUid(ModelId id) const
{
	assert(id != ModelId::Null);
	return data.model[id.i].hasUid ? Optional(data.model[id.i].uid) : Optional<Uid>();
}

ArrayView<const ModelMesh> ModelManager::GetModelMeshes(ModelId id) const
{
	assert(id != ModelId::Null);
	const ModelData& model = data.model[id.i];
	return ArrayView<const ModelMesh>(model.meshes, model.meshCount);
}

ArrayView<const ModelNode> ModelManager::GetModelNodes(ModelId id) const
{
	assert(id != ModelId::Null);
	const ModelData& model = data.model[id.i];
	return ArrayView<const ModelNode>(model.nodes, model.nodeCount);
}

ArrayView<const ModelMeshPart> ModelManager::GetModelMeshParts(ModelId id) const
{
	assert(id != ModelId::Null);
	const ModelData& model = data.model[id.i];
	return ArrayView<const ModelMeshPart>(model.meshParts, model.meshPartCount);
}

uint32_t ModelManager::AcquireSlot()
{
	uint32_t id;

	if (data.freelistFirst != 0)
	{
		id = data.freelistFirst;
		uint32_t nextFree = data.freelist[data.freelistFirst];
		data.freelist[data.freelistFirst] = 0;
		data.freelistFirst = nextFree;
	}
	else
	{
		if (data.slotsUsed == data.allocated)
			Reallocate(data.slotsUsed + 1);

		id = data.slotsUsed;
		data.slotsUsed += 1;
	}
	
	return id;
}

void ModelManager::ReleaseSlot(uint32_t id)
{
	// Add to freelist
	data.freelist[id] = data.freelistFirst;
	data.freelistFirst = id;
}

void ModelManager::Reallocate(uint32_t required)
{
	if (required <= data.allocated)
		return;

	required = static_cast<uint32_t>(Math::UpperPowerOfTwo(required));

	constexpr size_t bytesPerInstance = sizeof(data.freelist[0]) + sizeof(data.model[0]);

	InstanceData newData;
	newData.buffer = allocator->Allocate(bytesPerInstance * required, "ModelManager.data.buffer");
	newData.slotsUsed = data.slotsUsed;
	newData.allocated = required;

	newData.freelist = static_cast<uint32_t*>(newData.buffer);
	newData.model = reinterpret_cast<ModelData*>(newData.freelist + required);

	if (data.buffer == nullptr)
	{
		memset(newData.freelist, 0, sizeof(uint32_t) * required);
	}
	else
	{
		size_t freelistSize = data.slotsUsed * sizeof(uint32_t);
		memcpy(newData.freelist, data.freelist, freelistSize);
		memset(newData.freelist + data.slotsUsed, 0, sizeof(uint32_t) * required - freelistSize);
		memcpy(newData.model, data.model, data.slotsUsed * sizeof(ModelData));

		allocator->Deallocate(data.buffer);
	}

	data = newData;
}

void ModelManager::CreateRenderData(ModelData& model, Array<uint8_t>& geometryBuffer)
{
	renderDevice->CreateBuffers(1, &model.bufferId);

	uint32_t bufferSize = static_cast<uint32_t>(geometryBuffer.GetCount());
	renderDevice->SetBufferStorage(model.bufferId, bufferSize, geometryBuffer.GetData(), BufferStorageFlags::None);

	// Update buffers

	for (uint32_t meshIdx = 0; meshIdx < model.meshCount; ++meshIdx)
	{
		ModelMesh& mesh = model.meshes[meshIdx];

		uint16_t primIdx = mesh.partOffset, primEnd = mesh.partOffset + mesh.partCount;
		for (; primIdx != primEnd; ++primIdx)
		{
			ModelMeshPart& part = model.meshParts[primIdx];
			VertexFormat& vertexFormat = part.vertexFormat;

			assert(vertexFormat.attributes != nullptr && vertexFormat.attributeCount > 0);

			// Create vertex array object
			renderDevice->CreateVertexArrays(1, &part.vertexArrayId);
			kokko::render::VertexArrayId va = part.vertexArrayId;

			if (mesh.indexType != RenderIndexType::None)
				renderDevice->SetVertexArrayIndexBuffer(va, model.bufferId);

			for (uint32_t bindingIndex = 0; bindingIndex < vertexFormat.attributeCount; ++bindingIndex)
			{
				const VertexAttribute& attr = vertexFormat.attributes[bindingIndex];

				// TODO: Use a single binding point when attributes are interleaved
				renderDevice->SetVertexArrayVertexBuffer(va, bindingIndex, model.bufferId, attr.offset, attr.stride);
				renderDevice->EnableVertexAttribute(va, attr.attrIndex);
				renderDevice->SetVertexAttribFormat(va, attr.attrIndex, attr.elemCount, attr.elemType, 0);
				renderDevice->SetVertexAttribBinding(va, attr.attrIndex, bindingIndex);
			}
		}
	}
}

void ModelManager::ReleaseRenderData(ModelData& model)
{
	for (uint32_t partIndex = 0; partIndex != model.meshPartCount; ++partIndex)
	{
		renderDevice->DestroyVertexArrays(1, &model.meshParts[partIndex].vertexArrayId);
		model.meshParts[partIndex].vertexArrayId = render::VertexArrayId::Null;
	}

	if (model.bufferId != render::BufferId::Null)
	{
		renderDevice->DestroyBuffers(1, &model.bufferId);
		model.bufferId = render::BufferId::Null;
	}
}

}
