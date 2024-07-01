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

	struct SShadowViewRenderCommandData
	{
		SShadowmapViewData ShadowViewData;
	};

	struct SMultiShadowViewRenderCommandData
	{
		std::array<SShadowmapViewData, 6> ShadowmapViews; 
	};

	struct SVolumetricRenderCommandData
	{
		F32 NumberOfSamplesReciprocal;
		F32 LightPower;
		F32 ScatteringProbability;
		F32 HenyeyGreensteinGValue;
	};

	struct SColorRenderCommandData
	{
		SColor Color;
		F32 Intensity;
	};

	struct SStaticMeshRenderCommandData
	{
		std::string StaticMeshName;
		U8 TopologyIndex;
		std::vector<SDrawCallData> DrawCallData;
	};

	struct SViewRenderCommandData
	{
		SMatrix ObjectMatrix;
		SMatrix ProjectionMatrix;
	};

	struct SConeRenderCommandData
	{
		SVector4 Direction;
		SVector4 DirectionNormal1;
		SVector4 DirectionNormal2;
		F32 OuterAngle;
		F32 InnerAngle;
	};

	struct SRayRenderCommandData
	{
		SVector4 Direction;
		F32 Range;
	};

	struct STextureRenderCommandData
	{
		bool ShouldRenderAlbedo;
		bool ShouldRenderMaterial;
		bool ShouldRenderNormal;
		U32 TextureIndex;
		std::vector<U16> TextureReferences;
	};

	struct SAmbientRenderCommandData
	{
		U16 AmbientCubemapReference;
	};

	struct SMaterialRenderCommandData
	{
		U8 PixelShaderIndex;
		std::vector<SEngineGraphicsMaterial> Materials;
	};

	struct SSamplerRenderCommandData
	{
		U8 SamplerIndex;
	};

	struct SRenderCommand
	{
		ERenderCommandType Type;

		std::vector<SMatrix> Matrices;
		std::vector<SVector4> Vectors;
		std::vector<SColor> Colors;
		std::vector<F32> F32s;
		std::vector<U8> U8s;
		std::vector<U16> U16s;
		std::vector<U32> U32s;
		std::vector<bool> Flags;
		std::vector<std::string> Strings;
		std::vector<SEngineGraphicsMaterial> Materials;
		std::vector<SDrawCallData> DrawCallData;
		std::vector<SShadowmapViewData> ShadowmapViews;

		~SRenderCommand() = default;
	
		void SetShadowMapViews(const std::array<SShadowmapViewData, 6>& shadowmapViews)
		{
			ShadowmapViews.assign(shadowmapViews.begin(), shadowmapViews.end());
		}

		void SetVolumetricDataFromComponent(const SVolumetricLightComponent& component)
		{
			F32s.push_back(component.NumberOfSamples);
			F32s.push_back(component.LightPower);
			F32s.push_back(component.ScatteringProbability);
			F32s.push_back(component.HenyeyGreensteinGValue);
		}
	};
}