// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "ECS/ECSInclude.h"

namespace Havtorn
{
//#define GetComponent(x) GetComponentInternal<const S##x*>(EComponentType::##x)
//#define GetComponent(x) GetComponentInternal<S##x&>(EComponentType::##x)
#define GetComponent(x) Get##x()

	struct SComponent;

	// TODO.NR: Can't send pointers between threads (to Render Thread), need to copy components
	typedef std::array<const SComponent*, static_cast<U64>(EComponentType::Count)> SComponentArray;

	enum class ERenderCommandType
	{
		ShadowAtlasPrePassDirectional,
		ShadowAtlasPrePassPoint,
		ShadowAtlasPrePassSpot,
		CameraDataStorage,
		GBufferDataInstanced,
		GBufferSpriteInstanced,
		DecalDepthCopy,
		DeferredDecal,
		PreLightingPass,
		DeferredLightingDirectional,
		DeferredLightingPoint,
		DeferredLightingSpot,
		PostBaseLightingPass,
		VolumetricLightingDirectional,
		VolumetricLightingPoint,
		VolumetricLightingSpot,
		VolumetricBufferBlurPass,
		ForwardTransparency,
		OutlineMask,
		Outline,
		ScreenSpaceSprite,
		Bloom,
		Tonemapping,
		PreDebugShape,
		PostToneMappingUseDepth,
		DebugShapeUseDepth,
		PostToneMappingIgnoreDepth,
		DebugShapeIgnoreDepth,
		AntiAliasing,
		GammaCorrection,
		RendererDebug
	};

	struct SRenderCommand
	{
		//inline SRenderCommand(std::array<const SComponent*, static_cast<U64>(EComponentType::Count)> components, ERenderCommandType type)
		//	: Components(components)
		//	, Type(type) 
		//{}

		~SRenderCommand() = default;

		//template<typename T>
		//inline T operator[](EComponentType type)
		//{
		//	return dynamic_cast<T>(Components[static_cast<U64>(type)].get());
		//}

		//template<typename T>
		//inline T GetComponentInternal(EComponentType type) const
		//{
		//	//return 	dynamic_cast<T>(Components[static_cast<U64>(type)].get());
		//	return dynamic_cast<T>(Components[static_cast<U64>(type)]);
		//}

		//template<typename T>
		//inline T GetComponentInternal(EComponentType type) const
		//{
		//	switch (type)
		//	{
		//	case Havtorn::EComponentType::TransformComponent:
		//		return TransformComponent;
		//	case Havtorn::EComponentType::StaticMeshComponent:
		//		return StaticMeshComponent;
		//	case Havtorn::EComponentType::CameraComponent:
		//		return CameraComponent;
		//	case Havtorn::EComponentType::CameraControllerComponent:
		//		return CameraControllerComponent;
		//	case Havtorn::EComponentType::MaterialComponent:
		//		return MaterialComponent;
		//	case Havtorn::EComponentType::EnvironmentLightComponent:
		//		return EnvironmentLightComponent;
		//	case Havtorn::EComponentType::DirectionalLightComponent:
		//		return DirectionalLightComponent;
		//	case Havtorn::EComponentType::PointLightComponent:
		//		return PointLightComponent;
		//	case Havtorn::EComponentType::SpotLightComponent:
		//		return SpotLightComponent;
		//	case Havtorn::EComponentType::VolumetricLightComponent:
		//		return VolumetricLightComponent;
		//	case Havtorn::EComponentType::DecalComponent:
		//		return DecalComponent;
		//	case Havtorn::EComponentType::DebugShapeComponent:
		//		return DebugShapeComponent;
		//	case Havtorn::EComponentType::MetaDataComponent:
		//		return MetaDataComponent;
		//	case Havtorn::EComponentType::Count:
		//	default:
		//		return TransformComponent;
		//	}
		//}

		inline const STransformComponent& GetTransformComponent() const { return TransformComponent; }
		inline const SStaticMeshComponent& GetStaticMeshComponent() const { return StaticMeshComponent; }
		inline const SCameraComponent& GetCameraComponent() const { return CameraComponent; }
		inline const SCameraControllerComponent& GetCameraControllerComponent() const { return CameraControllerComponent; }
		inline const SMaterialComponent& GetMaterialComponent() const { return MaterialComponent; }
		inline const SEnvironmentLightComponent& GetEnvironmentLightComponent() const { return EnvironmentLightComponent; }
		inline const SDirectionalLightComponent& GetDirectionalLightComponent() const { return DirectionalLightComponent; }
		inline const SPointLightComponent& GetPointLightComponent() const { return PointLightComponent; }
		inline const SSpotLightComponent& GetSpotLightComponent() const { return SpotLightComponent; }
		inline const SVolumetricLightComponent& GetVolumetricLightComponent() const { return VolumetricLightComponent; }
		inline const SDecalComponent& GetDecalComponent() const { return DecalComponent; }
		inline const SSpriteComponent& GetSpriteComponent() const { return SpriteComponent; }
		inline const STransform2DComponent& GetTransform2DComponent() const { return Transform2DComponent; }
		inline const SDebugShapeComponent& GetDebugShapeComponent() const { return DebugShapeComponent; }
		inline const SMetaDataComponent& GetMetaDataComponent() const { return MetaDataComponent; }

		//SComponentArray Components;
		ERenderCommandType Type;
		STransformComponent TransformComponent;
		SStaticMeshComponent StaticMeshComponent;
		SCameraComponent CameraComponent;
		SCameraControllerComponent CameraControllerComponent;
		SMaterialComponent MaterialComponent;
		SEnvironmentLightComponent EnvironmentLightComponent;
		SDirectionalLightComponent DirectionalLightComponent;
		SPointLightComponent PointLightComponent;
		SSpotLightComponent SpotLightComponent;
		SVolumetricLightComponent VolumetricLightComponent;
		SDecalComponent DecalComponent;
		SSpriteComponent SpriteComponent;
		STransform2DComponent Transform2DComponent;
		SDebugShapeComponent DebugShapeComponent;
		SMetaDataComponent MetaDataComponent;
	};
}
