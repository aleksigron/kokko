#pragma once

#include "yaml-cpp/yaml.h"

#include "Core/Core.hpp"
#include "Core/Hash.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/Entity.hpp"

#include "Rendering/CameraSystem.hpp"

class CameraSerializer final : public ComponentSerializer
{
public:
	CameraSerializer(CameraSystem* cameraSystem) :
		cameraSystem(cameraSystem)
	{
	}

	virtual uint32_t GetComponentTypeNameHash() override
	{
		return "camera"_hash;
	}

	virtual void DeserializeComponent(const YAML::Node& map, Entity entity) override
	{
		ProjectionParameters params;
		params.projection = ProjectionType::Perspective;
		params.perspectiveFieldOfView = 1.0f;
		params.perspectiveNear = 0.1f;
		params.perspectiveFar = 100.0f;
		params.orthographicHeight = 2.0f;
		params.orthographicNear = 0.0f;
		params.orthographicFar = 10.0f;
		params.SetAspectRatio(16.0f, 9.0f);

		YAML::Node typeNode = map["projection_type"];
		if (typeNode.IsDefined() && typeNode.IsScalar())
		{
			const std::string& typeStr = typeNode.Scalar();
			uint32_t typeHash = kokko::HashString(typeStr.data(), typeStr.size());

			switch (typeHash)
			{
			case "perspective"_hash:
				params.projection = ProjectionType::Perspective;
				break;

			case "orthographic"_hash:
				params.projection = ProjectionType::Orthographic;
				break;

			default:
				KK_LOG_ERROR("LevelLoader: Unknown projection type");
				return;
			}
		}
		else
		{
			KK_LOG_ERROR("LevelLoader: Projection type not specified");
			return;
		}

		YAML::Node fovNode = map["field_of_view"];
		if (fovNode.IsDefined() && fovNode.IsScalar())
			params.perspectiveFieldOfView = fovNode.as<float>();

		YAML::Node heightNode = map["orthographic_height"];
		if (heightNode.IsDefined() && heightNode.IsScalar())
			params.orthographicHeight = heightNode.as<float>();

		YAML::Node nearNode = map["near"];
		if (nearNode.IsDefined() && nearNode.IsScalar())
		{
			float near = nearNode.as<float>();

			if (params.projection == ProjectionType::Perspective)
				params.perspectiveNear = near;
			else
				params.orthographicNear = near;
		}

		YAML::Node farNode = map["far"];
		if (farNode.IsDefined() && farNode.IsScalar())
		{
			float far = farNode.as<float>();

			if (params.projection == ProjectionType::Perspective)
				params.perspectiveFar = far;
			else
				params.orthographicFar = far;
		}

		CameraId cameraId = cameraSystem->AddComponentToEntity(entity);
		cameraSystem->SetData(cameraId, params);
	}

	virtual void SerializeComponent(YAML::Emitter& out, Entity entity) override
	{
		CameraId cameraId = cameraSystem->Lookup(entity);
		if (cameraId != CameraId::Null())
		{
			out << YAML::BeginMap;
			out << YAML::Key << GetComponentTypeKey() << YAML::Value << "camera";

			ProjectionParameters params = cameraSystem->GetData(cameraId);

			out << YAML::Key << "projection_type" << YAML::Value << CameraSystem::GetProjectionTypeName(params.projection);

			if (params.projection == ProjectionType::Perspective)
			{
				out << YAML::Key << "field_of_view" << YAML::Value << params.perspectiveFieldOfView;
				out << YAML::Key << "near" << YAML::Value << params.perspectiveNear;
				out << YAML::Key << "far" << YAML::Value << params.perspectiveFar;
			}
			else
			{
				out << YAML::Key << "orthographic_height" << YAML::Value << params.orthographicHeight;
				out << YAML::Key << "near" << YAML::Value << params.orthographicNear;
				out << YAML::Key << "far" << YAML::Value << params.orthographicFar;
			}

			out << YAML::EndMap;
		}
	}

private:
	CameraSystem* cameraSystem;
};
