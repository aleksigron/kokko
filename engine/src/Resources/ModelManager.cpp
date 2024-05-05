#include "Resources/ModelManager.hpp"

#include "Core/Array.hpp"
#include "Core/Core.hpp"

#include "Resources/AssetLoader.hpp"
#include "Resources/ModelLoader.hpp"
#include "Resources/MeshId.hpp"

#include "Rendering/RenderDevice.hpp"

namespace kokko
{

const ModelId ModelId::Null = ModelId{ 0 };

ModelManager::ModelManager(Allocator* allocator, AssetLoader* assetLoader, render::Device* renderDevice) :
	allocator(allocator),
	assetLoader(assetLoader),
	renderDevice(renderDevice),
	uidMap(allocator),
	models(allocator)
{
	models.Reserve(32);
	models.PushBack(ModelData{}); // Reserve index 0 for ModelId::Null
}

ModelManager::~ModelManager()
{
	for (size_t i = 1, count = models.GetCount(); i < count; ++i)
	{
		allocator->Deallocate(models[i].buffer);
	}
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
		unsigned int id = static_cast<unsigned int>(models.GetCount());
		ModelData& model = models.PushBack();
		model.uid = uid;

		Array<uint8_t> geometryBuffer(allocator);
		ModelLoader loader(allocator);

		if (loader.LoadGlbFromBuffer(&model, &geometryBuffer, file.GetView()))
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
			models.PopBack();
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

kokko::Uid ModelManager::GetModelUid(ModelId id) const
{
	return models[id.i].uid;
}

ArrayView<const ModelMesh> ModelManager::GetModelMeshes(ModelId id) const
{
	const ModelData& model = models[id.i];
	return ArrayView<const ModelMesh>(model.meshes, model.meshCount);
}

ArrayView<const ModelNode> ModelManager::GetModelNodes(ModelId id) const
{
	const ModelData& model = models[id.i];
	return ArrayView<const ModelNode>(model.nodes, model.nodeCount);
}

ArrayView<const ModelPrimitive> ModelManager::GetModelPrimitives(ModelId id) const
{
	const ModelData& model = models[id.i];
	return ArrayView<const ModelPrimitive>(model.primitives, model.primitiveCount);
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

		// TODO
		// data.uniqueVertexCount[index] = static_cast<int>(vdata.vertexCount);

		for (uint16_t primIdx = mesh.primitiveOffset, primEnd = mesh.primitiveOffset + mesh.primitiveCount; primIdx < primEnd; ++primIdx)
		{
			ModelPrimitive& primitive = model.primitives[primIdx];
			VertexFormat& vertexFormat = primitive.vertexFormat;

			assert(vertexFormat.attributes != nullptr && vertexFormat.attributeCount > 0);

			// Create vertex array object
			renderDevice->CreateVertexArrays(1, &primitive.vertexArrayId);
			kokko::render::VertexArrayId va = primitive.vertexArrayId;

			// TODO: Only if indexed
			renderDevice->SetVertexArrayIndexBuffer(va, model.bufferId);

			for (unsigned int i = 0; i < vertexFormat.attributeCount; ++i)
			{
				const VertexAttribute& attr = vertexFormat.attributes[i];

				renderDevice->SetVertexArrayVertexBuffer(va, i, model.bufferId, attr.offset, attr.stride);
				renderDevice->EnableVertexAttribute(va, attr.attrIndex);
				renderDevice->SetVertexAttribBinding(va, attr.attrIndex, i);
				renderDevice->SetVertexAttribFormat(va, attr.attrIndex, attr.elemCount, attr.elemType, 0);
			}
		}
	}
}

}
