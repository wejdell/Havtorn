// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	struct SSpotLightComponent : public SComponent
	{
		SSpotLightComponent() = default;
		SSpotLightComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		bool IsActive = false;
		SShadowmapViewData ShadowmapView = {};
		SVector4 ColorAndIntensity = SVector4(1.0f, 1.0f, 1.0f, 1.0f);
		SVector4 Direction = SVector4::Forward;
		SVector4 DirectionNormal1 = SVector4::Right;
		SVector4 DirectionNormal2 = SVector4::Up;
		SVector2<F32> ShadowViewSize = { 8.0f, 8.0f };
		F32 OuterAngle = 90.0f;
		F32 InnerAngle = 45.0f;
		F32 Range = 1.0f;

		// TODO.NR: If we assume that EEditorResourceTexture won't change order (because we don't expect to have many editor resource textures), this
		// hard coded index works perfectly fine. We might want to find a cleaner abstraction or way to connect this editor data to the engine though.
		U32 EditorTextureIndex = 10;
	};
}
