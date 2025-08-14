// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct SEnvironmentLightComponent : public SComponent
	{
		SEnvironmentLightComponent() = default;
		SEnvironmentLightComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		bool IsActive = false;
		U16 AmbientCubemapReference = 0;
		U64 AssetRegistryKey = 0;
	
		// TODO.NR: If we assume that EEditorResourceTexture won't change order (because we don't expect to have many editor resource textures), this
		// hard coded index works perfectly fine. We might want to find a cleaner abstraction or way to connect this editor data to the engine though.
		U32 EditorTextureIndex = 7;
	};
}
