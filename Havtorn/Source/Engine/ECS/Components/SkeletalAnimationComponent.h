// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct SSkeletalAnimationComponent : public SComponent
	{
		SSkeletalAnimationComponent() = default;
		SSkeletalAnimationComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		SVector2<U32> AnimationData;
	};
}
