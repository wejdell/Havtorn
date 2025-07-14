// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	struct SDirectionalLightComponent : public SComponent
	{
		SDirectionalLightComponent() = default;
		SDirectionalLightComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		bool IsActive = true;
		SShadowmapViewData ShadowmapView = SShadowmapViewData();
		SVector4 Direction = SVector4::Up;
		SVector4 Color = SVector4(1.0f, 1.0f, 1.0f, 1.0f);
		SVector2<F32> ShadowViewSize = { 8.0f, 8.0f };
		SVector2<F32> ShadowNearAndFarPlane = { -8.0f, 8.0f };
		
		// TODO.NR: If we assume that EEditorResourceTexture won't change order (because we don't expect to have many editor resource textures), this
		// hard coded index works perfectly fine. We might want to find a cleaner abstraction or way to connect this editor data to the engine though.
		U32 EditorTextureIndex = 8;
	};
}
