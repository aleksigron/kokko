#pragma once

#include "ryml.hpp"

#include "Engine/Entity.hpp"

#include "Graphics/Scene.hpp"

#include "Resources/YamlCustomTypes.hpp"

namespace kokko
{

class TransformSerializer
{
private:
	Scene* scene;

public:
	TransformSerializer(Scene* scene) : scene(scene)
	{
	}

	void Serialize(c4::yml::NodeRef& componentArray, Entity entity, SceneObjectId sceneObj)
	{
		if (sceneObj != SceneObjectId::Null)
		{
			const SceneEditTransform& transform = scene->GetEditTransform(sceneObj);

			ryml::NodeRef componentNode = componentArray.append_child();
			componentNode |= ryml::MAP;
			componentNode["component_type"] = "transform";
			componentNode["position"] << transform.translation;
			componentNode["rotation"] << transform.rotation;
			componentNode["scale"] << transform.scale;
		}
	}

	SceneObjectId Deserialize(const c4::yml::ConstNodeRef& map, Entity entity, SceneObjectId parent)
	{
		SceneObjectId sceneObject = scene->AddSceneObject(entity);

		if (parent != SceneObjectId::Null)
			scene->SetParent(sceneObject, parent);

		SceneEditTransform transform;

		auto positionNode = map.find_child("position");
		if (positionNode.valid() && positionNode.has_val())
			positionNode >> transform.translation;

		auto rotationNode = map.find_child("rotation");
		if (rotationNode.valid() && rotationNode.has_val())
			rotationNode >> transform.rotation;

		auto scaleNode = map.find_child("scale");
		if (scaleNode.valid() && scaleNode.has_val())
			scaleNode >> transform.scale;

		scene->SetEditTransform(sceneObject, transform);

		return sceneObject;
	}
};

} // namespace kokko
