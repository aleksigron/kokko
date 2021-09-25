#pragma once

#include "yaml-cpp/yaml.h"

#include "Engine/Entity.hpp"

#include "Graphics/Scene.hpp"

#include "Resources/YamlCustomTypes.hpp"

class TransformSerializer
{
private:
	Scene* scene;

public:
	TransformSerializer(Scene* scene) : scene(scene)
	{
	}

	void Serialize(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj)
	{
		if (sceneObj != SceneObjectId::Null)
		{
			const SceneEditTransform& transform = scene->GetEditTransform(sceneObj);

			out << YAML::BeginMap;
			out << YAML::Key << "component_type" << YAML::Value << "transform";
			out << YAML::Key << "position" << YAML::Value << transform.translation;
			out << YAML::Key << "rotation" << YAML::Value << transform.rotation;
			out << YAML::Key << "scale" << YAML::Value << transform.scale;
			out << YAML::EndMap;
		}
	}

	SceneObjectId Deserialize(const YAML::Node& map, Entity entity, SceneObjectId parent)
	{
		SceneObjectId sceneObject = scene->AddSceneObject(entity);

		if (parent != SceneObjectId::Null)
			scene->SetParent(sceneObject, parent);

		SceneEditTransform transform;

		YAML::Node positionNode = map["position"];
		if (positionNode.IsDefined() && positionNode.IsSequence())
			transform.translation = positionNode.as<Vec3f>();

		YAML::Node rotationNode = map["rotation"];
		if (rotationNode.IsDefined() && rotationNode.IsSequence())
			transform.rotation = rotationNode.as<Vec3f>();

		YAML::Node scaleNode = map["scale"];
		if (scaleNode.IsDefined() && scaleNode.IsSequence())
			transform.scale = scaleNode.as<Vec3f>();

		scene->SetEditTransform(sceneObject, transform);

		return sceneObject;
	}
};
