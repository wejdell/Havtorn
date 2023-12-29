// Copyright 2023 Team Havtorn. All Rights Reserved.

// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"
#include "ECS/Components/SequencerComponent.h"

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

	class CSequencerSystem final : public ISystem
	{
	public:
		CSequencerSystem() = default;
		~CSequencerSystem() override = default;

		void Update(CScene* scene) override;

		HAVTORN_API SSequencerContextData GetSequencerContextData() const;
		HAVTORN_API void SetSequencerContextData(const SSequencerContextData& data);	
		HAVTORN_API void AddComponentTrackToComponent(SSequencerComponent& sequencerComponent, EComponentType trackComponentType);

		template<typename T>
		T* AddEmptyKeyframeToComponent(SSequencerComponent& sequencerComponent, EComponentType componentType);
	
		HAVTORN_API void RecordNewKeyframes(CScene* scene, std::vector<SSequencerComponent>& sequencerComponents);
	private:
		void Tick(CScene* scene, std::vector<SSequencerComponent>& sequencerComponents);
		void UpdateTracks(CScene* scene, std::vector<SSequencerComponent>& sequencerComponents);
		void OnSequenceFinished();

	private:
		SSequencerContextData Data = {};
		U32 InternalCurrentFrame = 0;
		F32 TickTime = 0.0f;
		bool ShouldRecordNewKeyframes = false;
		bool IsInitialized = false;
	};

	template<typename T>
	T* CSequencerSystem::AddEmptyKeyframeToComponent(SSequencerComponent& sequencerComponent, EComponentType componentType)
	{
		// TODO.NR: Add new component track if doesn't exist?

		for (SSequencerComponentTrack& componentTrack : sequencerComponent.ComponentTracks)
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

				return newKeyframe;
			}
		}

		return nullptr;
	}
}