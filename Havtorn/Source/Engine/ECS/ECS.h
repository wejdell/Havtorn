// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	enum class EComponentType
	{
		TransformComponent,
		StaticMeshComponent,
		CameraComponent,
		CameraControllerComponent,
		MaterialComponent,
		EnvironmentLightComponent,
		DirectionalLightComponent,
		PointLightComponent,
		SpotLightComponent,
		VolumetricLightComponent,
		DecalComponent,
		SpriteComponent,
		Transform2DComponent,
		SpriteAnimatorGraphComponent,
		DebugShapeComponent,
		MetaDataComponent,
		Count
	};

	static std::string GetComponentTypeString(EComponentType componentType)
	{
		switch (componentType)
		{
		case Havtorn::EComponentType::TransformComponent:
			return "TransformComponent";
		case Havtorn::EComponentType::StaticMeshComponent:
			return "StaticMeshComponent";
		case Havtorn::EComponentType::CameraComponent:
			return "CameraComponent";
		case Havtorn::EComponentType::CameraControllerComponent:
			return "CameraControllerComponent";
		case Havtorn::EComponentType::MaterialComponent:
			return "MaterialComponent";
		case Havtorn::EComponentType::EnvironmentLightComponent:
			return "EnvironmentLightComponent";
		case Havtorn::EComponentType::DirectionalLightComponent:
			return "DirectionalLightComponent";
		case Havtorn::EComponentType::PointLightComponent:
			return "PointLightComponent";
		case Havtorn::EComponentType::SpotLightComponent:
			return "SpotLightComponent";
		case Havtorn::EComponentType::VolumetricLightComponent:
			return "VolumetricLightComponent";
		case Havtorn::EComponentType::DecalComponent:
			return "DecalComponent";
		case Havtorn::EComponentType::SpriteComponent:
			return "SpriteComponent";
		case Havtorn::EComponentType::Transform2DComponent:
			return "Transform2DComponent";
		case Havtorn::EComponentType::SpriteAnimatorGraphComponent:
			return "SpriteAnimatorGraphComponent";

		case Havtorn::EComponentType::DebugShapeComponent:
		case Havtorn::EComponentType::MetaDataComponent:
		case Havtorn::EComponentType::Count:
		default:
			return "";
		}
	}
}
