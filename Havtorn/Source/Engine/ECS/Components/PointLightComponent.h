// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	struct SPointLightComponent : public SComponent
	{
		SPointLightComponent() = default;
		SPointLightComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		bool IsActive = true;
		std::array<SShadowmapViewData, 6> ShadowmapViews = {};
		SVector4 ColorAndIntensity = SVector4(1.0f, 1.0f, 1.0f, 1.0f);
		F32 Range = 1.0f;

		// TODO.NR: If we assume that EEditorResourceTexture won't change order (because we don't expect to have many editor resource textures), this
		// hard coded index works perfectly fine. We might want to find a cleaner abstraction or way to connect this editor data to the engine though.
		U32 EditorTextureIndex = 9;
	};
}
