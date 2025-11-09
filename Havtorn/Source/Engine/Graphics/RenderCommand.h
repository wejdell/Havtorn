// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "ECS/ECSInclude.h"

#include "GraphicsStructs.h"
#include "RenderingPrimitives/RenderTexture.h"

namespace Havtorn
{
	struct SComponent;

	enum class ERenderCommandType
	{
		ShadowAtlasPrePassDirectional,
		ShadowAtlasPrePassPoint,
		ShadowAtlasPrePassSpot,
		CameraDataStorage,
		GBufferDataInstanced,
		GBufferDataInstancedEditor,
		GBufferSkeletalInstanced,
		GBufferSkeletalInstancedEditor,
		GBufferSpriteInstanced,
		GBufferSpriteInstancedEditor,
		DecalDepthCopy,
		DeferredDecal,
		PreLightingPass,
		DeferredLightingDirectional,
		DeferredLightingPoint,
		DeferredLightingSpot,
		Skybox,
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
		WorldSpaceSpriteEditorWidget,
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

		std::vector<SMatrix> Matrices;
		std::vector<SVector4> Vectors;
		std::vector<SColor> Colors;
		std::vector<F32> F32s;
		std::vector<U8> U8s;
		std::vector<U16> U16s;
		std::vector<U32> U32s;
		std::vector<bool> Flags;
		std::vector<std::string> Strings;
		std::vector<SDrawCallData> DrawCallData;
		std::vector<CStaticRenderTexture> RenderTextures;
		std::vector<SShadowmapViewData> ShadowmapViews;
		std::vector<SEngineGraphicsMaterial> Materials;
		std::vector<std::map<U32, CStaticRenderTexture>> MaterialRenderTextures;
		U64 RenderViewID = 0;

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