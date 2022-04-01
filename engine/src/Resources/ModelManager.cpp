#include "Resources/ModelManager.hpp"

#include "Core/Array.hpp"
#include "Core/Core.hpp"

#include "Resources/AssetLoader.hpp"
#include "Resources/ModelLoader.hpp"
#include "Resources/MeshUid.hpp"

namespace
{

} // Anonymous namespace

namespace kokko
{

const ModelId ModelId::Null = ModelId{ 0 };

ModelManager::ModelManager(Allocator* allocator, AssetLoader* assetLoader, MeshManager* meshManager) :
	allocator(allocator),
	assetLoader(assetLoader),
	meshManager(meshManager),
	uidMap(allocator),
	models(allocator)
{
	models.Reserve(32);
	models.PushBack(ModelData{}); // Reserve index 0 for ModelId::Null
}

ModelManager::~ModelManager()
{
	// TODO: Release model data buffers
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

		ModelLoader loader(this);

		if (loader.LoadFromBuffer(model, file.GetView()))
		{
			pair = uidMap.Insert(uid);
			pair->second = id;

			return ModelId{ id };
		}
		else
		{
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


}
