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
		// NR: Not implemented yet, but should be about here in the list when we implement outlines for editor selections.
		//OutlineMask,
		//Outline,
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

		/*
		Will there (possibly) be more things that are not component-based that want to send RenderCommands?
		yes - suggestions/currently
		* DebugSystem

		Do we want Havtorn to support RenderCommands that are not tied to Components?
		(I say): yes. There can be times where users will want to send a custom RenderCommand that is not part of the ECS.
		Also, everytime a new Component is created (currently) the component would have to be added to to the RenderCommand-struct.
		Which might be tedious...? Having generalised data (even if not used with all commands) would open up some extra doors. 

		Would be nice to decouple RenderCommands from Components. Just have data.
		E.g data used by RenderManager exists in command.
		*	SDrawCallData is used in several Render-methods in RenderManager
		*	Instead of:
		FrameBufferData.ToCameraFromWorld = spotLightComp.ShadowmapView.ShadowViewMatrix;
		FrameBufferData.ToWorldFromCamera = spotLightComp.ShadowmapView.ShadowViewMatrix.FastInverse();
		FrameBufferData.ToProjectionFromCamera = spotLightComp.ShadowmapView.ShadowProjectionMatrix;
		FrameBufferData.ToCameraFromProjection = spotLightComp.ShadowmapView.ShadowProjectionMatrix.Inverse();
		FrameBufferData.CameraPosition = spotLightComp.ShadowmapView.ShadowPosition;
		It could be:
		FrameBufferData.ToCameraFromWorld = command.ToCameraFromWorld;
		FrameBufferData.ToWorldFromCamera = command.ToWorldFromCamera;
		FrameBufferData.ToProjectionFromCamera = command.ToProjectionFromCamera;
		FrameBufferData // ...

		I.e: the data of a RenderCommand is already interpreted and the RenderManger only fetches the data it needs in the method
		linked to by the type.

		// [Currently] Decal specific
		bool ShouldRenderAlbedo
		bool ShouldRenderMaterial
		bool ShouldRenderNormal

		SMatrix ToCameraFromWorld
		SMatrix ToWorldFromCamera
		SMatrix ToProjectionFromCamera
		SMatrix ToCameraFromProjection
		SMatrix ToWorldFromObject

		SMatrix ToWorld
		SMatrix ToObjectSpace

		SVector4 CameraPosition

		string staticMeshName

		F32 Thickness

		SVector4 Direction
		SVector4 DirectionNormal1
		SVector4 DirectionNormal2
		SColor Color
		F32 Intensity
		F32 Range
		F32 OuterAngle
		F32 InnerAngle

		// Volumetric data
		F32 NumberOfSmaplesReciprocal
		F32 LightPower 
		F32 ScatteringProbability
		F32 HenyeyGreensteinGValue

		U8 TopologyIndex
		U8 PixelShaderIndex
		U8 SamplerIndex
		std::vector<U32> TextureBankIndices

		SDrawCallData
		U32 IndexCount
		U16 VertexBufferIndex
		U16 IndexBufferIndex
		U16 VertexStrideIndex
		U16 VertexOffsetIndex

		std::vector<SShadowMapViewData>
		SMatrix ShadowViewMatrix
		SMatrix ShadowProjectionMatrix
		SVector4 ShadowPosition
		U16 ShadowmapViewportIndex

		SVector2 ShadowmapResolution
		SVector2 ShadowAtlasResolution
		SVector2 ShadowmapStartingUV
		F32	ShadowTestTolerance

		F32 EmissiveStrength

		*/

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
