// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/System.h"
#include "Assets/SequencerAsset.h"

namespace Havtorn
{
	struct SSequencerKeyframe;
	struct SSequencerEntityReference;

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
		HAVTORN_API void AddComponentTrackToEntityReference(const U64 guid, EComponentType trackComponentType);

		template<typename T>
		T* AddEmptyKeyframeToComponent(const U64 guid, EComponentType componentType);
	
		HAVTORN_API const EComponentType GetComponentTrackTypeFromEntityReference(const U64 guid, const I32 componentTrackIndex);
		HAVTORN_API SSequencerKeyframe* const GetKeyframeFromEntityReference(const U64 guid, const I32 componentTrackIndex, const I32 keyframeIndex);

		HAVTORN_API void RecordNewKeyframes(CScene* scene);
		HAVTORN_API void SortKeyframes(const U64 guid, U32 trackIndex, I32& lastEditedKeyframeIndex);

		HAVTORN_API const char* GetCurrentSequencerName() const;
		HAVTORN_API const std::vector<SSequencerEntityReference>* GetCurrentEntityReferences() const;

		HAVTORN_API void SaveCurrentSequencer(const std::string& filePath);
		HAVTORN_API void LoadSequencerFromFile(const std::string& filePath);
		HAVTORN_API void LoadSequencer(const std::string& sequencerName);

		HAVTORN_API CSequencerAsset* TryGetSequencerWithName(const std::string& sequencerName) const;

	private:
		void Tick(CScene* scene);
		void UpdateTracks(CScene* scene);
		void OnSequenceFinished();

	private:
		std::vector<CSequencerAsset*> SequencerAssets;
		SSequencerContextData Data = {};
		CSequencerAsset* CurrentSequencer = nullptr; // TODO.NR: Support multiple sequencers at the same time? Might be overkill for now
		U32 InternalCurrentFrame = 0;
		F32 TickTime = 0.0f;
		bool ShouldRecordNewKeyframes = false;
		bool IsInitialized = false;
	};

	template<typename T>
	T* CSequencerSystem::AddEmptyKeyframeToComponent(const U64 guid, EComponentType componentType)
	{
		// TODO.NR: Add new component track if doesn't exist?

		if (!CurrentSequencer)
			return nullptr;

		SSequencerEntityReference* const entityReferencePointer = CurrentSequencer->TryGetEntityReference(guid);
		if (!entityReferencePointer)
			return nullptr;

		for (SSequencerComponentTrack& componentTrack : entityReferencePointer->ComponentTracks)
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