// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
#define GetComponent(x) GetComponentInternal<const S##x*>(EComponentType::##x)

	struct SComponent;

	typedef std::array<const SComponent*, static_cast<U64>(EComponentType::Count)> SComponentArray;

	enum class ERenderCommandType
	{
		ShadowAtlasPrePassDirectional,
		ShadowAtlasPrePassPoint,
		ShadowAtlasPrePassSpot,
		CameraDataStorage,
		GBufferDataInstanced,
		DecalDepthCopy,
		DeferredDecal,
		PreLightingPass,
		DeferredLightingDirectional,
		DeferredLightingPoint,
		DeferredLightingSpot,
		VolumetricLightingDirectional,
		VolumetricLightingPoint,
		VolumetricLightingSpot,
		VolumetricBufferBlurPass,
		ForwardTransparency,
		OutlineMask,
		Outline,
		Bloom,
		Tonemapping,
		DebugShape,
		AntiAliasing,
		GammaCorrection,
		RendererDebug
	};

	struct SRenderCommand
	{
		inline SRenderCommand(std::array<const SComponent*, static_cast<U64>(EComponentType::Count)> components, ERenderCommandType type)
			: Components(components)
			, Type(type) 
		{}

		~SRenderCommand() = default;

		template<typename T>
		inline T operator[](EComponentType type)
		{
			return dynamic_cast<T>(Components[static_cast<U64>(type)].get());
		}

		template<typename T>
		inline T GetComponentInternal(EComponentType type) const
		{
			//return 	dynamic_cast<T>(Components[static_cast<U64>(type)].get());
			return dynamic_cast<T>(Components[static_cast<U64>(type)]);
		}

		SComponentArray Components;
		ERenderCommandType Type;
	};
}
