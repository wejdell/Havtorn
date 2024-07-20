// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Core/Color.h"

namespace Havtorn
{
	struct SSpriteComponent : public SComponent
	{
		SSpriteComponent() = default;
		SSpriteComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		SColor Color = SColor::White;
		SVector4 UVRect = SVector4::Zero;
		U32 TextureIndex = 0;
		U64 AssetRegistryKey = 0;
	};
}