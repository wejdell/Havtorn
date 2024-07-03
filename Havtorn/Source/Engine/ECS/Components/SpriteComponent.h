// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Core/Color.h"

namespace Havtorn
{
	struct SSpriteComponent : public SComponent
	{
		SSpriteComponent()
			: SComponent(EComponentType::SpriteComponent)
		{}

		SColor Color = SColor::White;
		SVector4 UVRect = SVector4::Zero;
		U32 TextureIndex = 0;
	};
}