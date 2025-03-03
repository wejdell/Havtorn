// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include <Color.h>

namespace Havtorn
{
	struct SSpriteComponent : public SComponent
	{
		SSpriteComponent() = default;
		SSpriteComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		SColor Color = SColor::White;
		SVector4 UVRect = { 0.f, 0.f, 1.f, 1.f };
		U64 AssetRegistryKey = 0;
		U16 TextureIndex = 0;
	};
}