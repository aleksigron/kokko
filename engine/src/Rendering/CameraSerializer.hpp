#pragma once

#include "ryml.hpp"

#include "Core/Core.hpp"
#include "Core/Hash.hpp"

#include "Engine/ComponentSerializer.hpp"
#include "Engine/Entity.hpp"

#include "Rendering/CameraSystem.hpp"

namespace kokko
{

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

	virtual void DeserializeComponent(const c4::yml::ConstNodeRef& map, Entity entity) override
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

		auto typeNode = map.find_child("projection_type");
		if (typeNode.valid() && typeNode.has_val())
		{
			auto typeStr = typeNode.val();
			uint32_t typeHash = kokko::HashString(typeStr.str, typeStr.len);

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

		auto fovNode = map.find_child("field_of_view");
		if (fovNode.valid() && fovNode.has_val() && fovNode.val().is_real())
			fovNode >> params.perspectiveFieldOfView;

		auto heightNode = map.find_child("orthographic_height");
		if (heightNode.valid() && heightNode.has_val() && heightNode.val().is_real())
			heightNode >> params.orthographicHeight;

		auto nearNode = map.find_child("near");
		if (nearNode.valid() && nearNode.has_val() && nearNode.val().is_real())
		{
			float near;
			nearNode >> near;

			if (params.projection == ProjectionType::Perspective)
				params.perspectiveNear = near;
			else
				params.orthographicNear = near;
		}

		auto farNode = map.find_child("far");
		if (farNode.valid() && farNode.has_val() && farNode.val().is_real())
		{
			float far;
			farNode >> far;

			if (params.projection == ProjectionType::Perspective)
				params.perspectiveFar = far;
			else
				params.orthographicFar = far;
		}

		CameraId cameraId = cameraSystem->AddCamera(entity);
		cameraSystem->SetProjection(cameraId, params);

		auto exposureNode = map.find_child("exposure");
		if (exposureNode.valid() && exposureNode.has_val())
		{
			float exposure;
			exposureNode >> exposure;
			cameraSystem->SetExposure(cameraId, exposure);
		}
	}

	virtual void SerializeComponent(c4::yml::NodeRef& componentArray, Entity entity) override
	{
		CameraId cameraId = cameraSystem->Lookup(entity);
		if (cameraId != CameraId::Null)
		{
			ryml::NodeRef componentNode = componentArray.append_child();
			componentNode |= ryml::MAP;
			componentNode[GetComponentTypeKey()] = "camera";

			const ProjectionParameters& params = cameraSystem->GetProjection(cameraId);
			float exposure = cameraSystem->GetExposure(cameraId);

			componentNode["projection_type"] << CameraSystem::GetProjectionTypeName(params.projection);

			if (params.projection == ProjectionType::Perspective)
			{
				componentNode["field_of_view"] << params.perspectiveFieldOfView;
				componentNode["near"] << params.perspectiveNear;
				componentNode["far"] << params.perspectiveFar;
			}
			else
			{
				componentNode["orthographic_height"] << params.orthographicHeight;
				componentNode["near"] << params.orthographicNear;
				componentNode["far"] << params.orthographicFar;
			}

			componentNode["exposure"] << exposure;
		}
	}

private:
	CameraSystem* cameraSystem;
};

} // namespace kokko
