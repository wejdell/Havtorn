// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct SNodeComponent : public SComponent
	{
		SNodeComponent(Ref<SEntity> entity, EComponentType type)
			: SComponent(std::move(entity), type)
		{}
	
		F32 Time = 0.0f;
	};
}
