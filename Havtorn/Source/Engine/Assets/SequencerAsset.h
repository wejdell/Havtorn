// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core/HavtornString.h"
#include "SequencerKeyframes/SequencerKeyframe.h"

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
		SSequencerComponentTrack(EComponentType componentType);
		~SSequencerComponentTrack();

		EComponentType ComponentType = EComponentType::Count;
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
		std::vector<SSequencerComponentTrack> ComponentTracks;

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

		HAVTORN_API SSequencerEntityReference* TryGetEntityReference(const U64 guid);
		HAVTORN_API bool operator==(const CSequencerAsset& other) const;
		HAVTORN_API bool operator==(const std::string& otherName) const;
	
	private:
		CHavtornStaticString<255> Name;
		std::vector<SSequencerEntityReference> EntityReferences;
	};
}
