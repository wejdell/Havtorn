// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct SDecalComponent : public SComponent
	{
		SDecalComponent(Ref<SEntity> entity, EComponentType type)
			: SComponent(std::move(entity), type)
		{}

		std::vector<U16> TextureReferences = {0, 0, 0};
		bool ShouldRenderAlbedo = false;
		bool ShouldRenderMaterial = false;
		bool ShouldRenderNormal = false;
	};
}