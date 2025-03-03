// TODO.NR: Add support for sequencer with new ECS. Has to be done on a separate branch, when we need it.
// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

#include "SequencerKeyframes/SequencerKeyframe.h"
#include <HavtornString.h>

namespace Havtorn
{
	enum class ESequencerComponentTrackState
	{
		Waiting,
		Blending,
		Setting
	};

	struct SSequencerComponentTrack : ISerializable
	{
		SSequencerComponentTrack() = default;
		~SSequencerComponentTrack();

		std::vector<SSequencerKeyframe*> Keyframes = {};
		SSequencerKeyframe* CurrentKeyframe = nullptr;
		SSequencerKeyframe* NextKeyframe = nullptr;
		ESequencerComponentTrackState TrackState = ESequencerComponentTrackState::Waiting;
		I32 CurrentKeyframeIndex = -1;

		// Inherited via ISerializable
		virtual U32 GetSize() const override;
		virtual void Serialize(char* toData, U64& pointerPosition) const override;
		virtual void Deserialize(const char* fromData, U64& pointerPosition) override;

	private: 
		mutable U32 NumberOfKeyframes = 0;
	};

	struct SSequencerEntityReference : ISerializable
	{
		SSequencerEntityReference() = default;
		SSequencerEntityReference(const U64 guid);

		U64 GUID = 0;
		std::map<U64, U64> ComponentTypeIndices;
		std::vector<SSequencerComponentTrack*> ComponentTracks;

		const bool operator==(U64 guid)
		{
			return GUID == guid;
		}

		// Inherited via ISerializable
		virtual U32 GetSize() const override;
		virtual void Serialize(char* toData, U64& pointerPosition) const override;
		virtual void Deserialize(const char* fromData, U64& pointerPosition) override;
	
	private:
		mutable U32 NumberOfComponentTracks = 0;
	};

	class CSequencerAsset
	{
	public:
		friend class CSequencerSystem;
		CSequencerAsset()
		{}

		ENGINE_API SSequencerEntityReference* TryGetEntityReference(const U64 guid);
		ENGINE_API bool operator==(const CSequencerAsset& other) const;
		ENGINE_API bool operator==(const std::string& otherName) const;
	
	private:
		CHavtornStaticString<255> Name;
		std::vector<SSequencerEntityReference> EntityReferences;
	};
}
