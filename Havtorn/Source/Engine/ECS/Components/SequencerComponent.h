// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/Component.h"
#include "SequencerKeyframes/SequencerKeyframe.h"

namespace Havtorn
{
	enum class ESequencerComponentTrackState
	{
		Waiting,
		Blending,
		Setting
	};

	struct SSequencerComponentTrack
	{
		EComponentType ComponentType = EComponentType::Count;
		std::vector<SSequencerKeyframe*> Keyframes = {};
		SSequencerKeyframe* CurrentKeyframe = nullptr;
		SSequencerKeyframe* NextKeyframe = nullptr;
		ESequencerComponentTrackState TrackState = ESequencerComponentTrackState::Waiting;
		I32 CurrentKeyframeIndex = -1;
	};

	struct SSequencerComponent : public SComponent
	{
		SSequencerComponent()
			: SComponent(EComponentType::SequencerComponent)
		{}

		std::vector<SSequencerComponentTrack> ComponentTracks;
	};
}
