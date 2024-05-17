// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

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

	struct SSequencerEntityReference
	{
		SSequencerEntityReference(const U64 guid)
			: GUID(guid)
			, ComponentTracks({})
		{}

		U64 GUID = 0;
		std::vector<SSequencerComponentTrack> ComponentTracks;

		const bool operator==(U64 guid)
		{
			return GUID == guid;
		}
	};

	class CSequencerAsset
	{
		friend class CSequencerSystem;

		CSequencerAsset()
		{}

		HAVTORN_API SSequencerEntityReference* TryGetEntityReference(const U64 guid);

	private:
		std::vector<SSequencerEntityReference> EntityReferences;
	};
}
