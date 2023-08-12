// Copyright 2023 Team Havtorn. All Rights Reserved.

// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"

namespace Havtorn
{
	struct SSequencerKeyframe;

	struct SSequencerContextData
	{
		U32 MaxFrames = 100;
		U32 CurrentFrame = 0;
		U16 PlayRate = 30;
		bool IsPlayingSequence = false;
		bool IsLooping = false;
	};

	enum class ESequencerComponentTrackState
	{
		Waiting,
		Blending,
		Setting
	};

	struct SSequencerComponentTrack
	{
		EComponentType ComponentType = EComponentType::Count;
		std::vector<SSequencerKeyframe*> Keyframes;
		SSequencerKeyframe* CurrentKeyframe;
		SSequencerKeyframe* NextKeyframe;
		ESequencerComponentTrackState TrackState = ESequencerComponentTrackState::Waiting;
		I32 CurrentKeyframeIndex = -1;
	};

	// This is just a proxy for an entity, might as well move the data to a component
	struct SSequencerEntityTrack
	{
		U64 EntityGUID = 0;
		std::vector<SSequencerComponentTrack> ComponentTracks;
	};

	class CSequencerSystem final : public ISystem
	{
	public:
		CSequencerSystem() = default;
		~CSequencerSystem() override = default;

		void Update(CScene* scene) override;

		HAVTORN_API SSequencerContextData GetSequencerContextData() const;
		HAVTORN_API void SetSequencerContextData(const SSequencerContextData& data);
		
		template<typename T>
		void AddEmptyKeyframe(EComponentType componentType);
	
	private:
		void RecordNewKeyframes(CScene* scene);
		void Tick(CScene* scene);
		void UpdateTracks(CScene* scene);
		void OnSequenceFinished();

	private:
		// TODO.NR: Think about design of data that is held by this system. Maybe it should hold some of its own, it could be 
		// interesting to try a SequencerComponent as well though, with their own triggers. If they are their own entities,
		// they can be made more lightweight, and when the component is inspected, then the authoring tool can open.

		// The logic still needs to run in this system, but it could be a good idea to move all the data out into smaller
		// components on their own entities.

		// TODO.NR: Think about the serialization of this data. Should a System really hold data that needs serialization? Ideally not.

		// NR: SelectedSequencerComponent on entity level?
		std::vector<SSequencerEntityTrack> Tracks;
		SSequencerContextData Data;
		U32 InternalCurrentFrame = 0;
		F32 TickTime = 0.0f;
		bool ShouldRecordNewKeyframes = false;
	};

	template<typename T>
	void CSequencerSystem::AddEmptyKeyframe(EComponentType componentType)
	{
		for (SSequencerEntityTrack& entityTrack : Tracks)
		{
			for (SSequencerComponentTrack& componentTrack : entityTrack.ComponentTracks)
			{
				if (componentType == componentTrack.ComponentType)
				{
					T* newKeyframe = new T();
					componentTrack.Keyframes.push_back(newKeyframe);
					newKeyframe->FrameNumber = Data.CurrentFrame;
					newKeyframe->ShouldRecord = true;
					ShouldRecordNewKeyframes = true;

					std::sort(componentTrack.Keyframes.begin(), componentTrack.Keyframes.end(), [&](const SSequencerKeyframe* a, const SSequencerKeyframe* b)
						{
							return a->FrameNumber < b->FrameNumber;
						});
				}
			}
		}
	}
}