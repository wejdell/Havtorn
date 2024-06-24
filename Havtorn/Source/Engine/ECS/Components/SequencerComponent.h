// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/Component.h"
#include "SequencerKeyframes/SequencerKeyframe.h"

namespace Havtorn
{
	struct SSequencerComponent : public SComponent
	{
		SSequencerComponent()
			: SComponent(EComponentType::SequencerComponent)
		{}
	};
}
