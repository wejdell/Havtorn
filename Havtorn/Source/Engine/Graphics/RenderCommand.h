// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "ECS/ECSInclude.h"

#include "GraphicsStructs.h"

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
		ERenderCommandType Type;

		SMatrix ObjectMatrix;
		SMatrix ProjectionMatrix;

		SVector4 Direction;
		SVector4 DirectionNormal1;
		SVector4 DirectionNormal2;
		SColor Color;
		F32 Intensity;
		F32 Range;
		F32 OuterAngle;
		F32 InnerAngle;

		F32 NumberOfSamplesReciprocal;
		F32 LightPower;
		F32 ScatteringProbability;
		F32 HenyeyGreensteinGValue;

		U8 TopologyIndex;
		U8 PixelShaderIndex;
		U8 SamplerIndex;
		U16 VertexBufferIndex;
		U16 IndexBufferIndex;
		U16 VertexStrideIndex;
		U16 VertexOffsetIndex;
		U32 IndexCount;
		U32 TextureIndex;

		std::string StaticMeshName;
		std::array<SShadowmapViewData, 6> ShadowmapViews;
		SShadowmapViewData ShadowViewData;
		SVector2<F32> ShadowmapResolution;
		SVector2<F32> ShadowAtlasResolution;
		SVector2<F32> ShadowmapStartingUV;
		F32	ShadowTestTolerance;

		F32 EmissiveStrength;

		bool ShouldRenderAlbedo;
		bool ShouldRenderMaterial;
		bool ShouldRenderNormal;

		U16 AmbientCubemapReference;

		std::vector<U32> TextureBankIndices;
		std::vector<U16> TextureReferences;
		std::vector<SEngineGraphicsMaterial> Materials;
		std::vector<SDrawCallData> DrawCallData;

		~SRenderCommand() = default;

		void SetVolumetricDataFromComponent(const SVolumetricLightComponent& component)
		{
			NumberOfSamplesReciprocal = component.NumberOfSamples;
			LightPower = component.LightPower;
			ScatteringProbability = component.ScatteringProbability;
			HenyeyGreensteinGValue = component.HenyeyGreensteinGValue;
		}
	};
}