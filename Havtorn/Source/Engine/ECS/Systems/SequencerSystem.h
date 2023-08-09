// Copyright 2023 Team Havtorn. All Rights Reserved.

// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"
//#include "ECS/ECSInclude.h"
#include "ECS/Components/TransformComponent.h"

namespace Havtorn
{
	struct SSequencerContextData
	{
		U32 MaxFrames = 100;
		U32 CurrentFrame = 0;
		U16 PlayRate = 30;
		bool IsPlayingSequence = false;
		bool IsLooping = false;
	};

	struct SSequencerKeyframe
	{
		U32 FrameNumber = 0;

		//virtual void Blend(SSequencerKeyframe* /*nextKeyframe*/, F32 /*blendParam*/) {};
	};
	
	struct SSequencerTransformKeyframe : public SSequencerKeyframe
	{
		SMatrix KeyframedComponent;
		SMatrix IntermediateComponent;

		void Blend(const SSequencerTransformKeyframe& nextKeyframe, F32 blendParam);
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
		std::vector<SSequencerTransformKeyframe> Keyframes;
		SSequencerTransformKeyframe CurrentKeyframe;
		SSequencerTransformKeyframe NextKeyframe;
		ESequencerComponentTrackState TrackState = ESequencerComponentTrackState::Waiting;
	};

	struct SSequencerEntityTrack
	{
		U64 EntityGUID = 0;
		std::vector<SSequencerComponentTrack> ComponentTracks;
	};

	class CSequencerSystem final : public ISystem
	{
	public:
		CSequencerSystem();
		~CSequencerSystem() override = default;

		void Update(CScene* scene) override;

		HAVTORN_API SSequencerContextData GetSequencerContextData() const;
		HAVTORN_API void SetSequencerContextData(const SSequencerContextData& data);
	
	private:
		void Tick(CScene* scene);
		void UpdateTracks(CScene* scene);
		void OnSequenceFinished();

	private:
		// TODO.NR: Think about design of data that is held by this system. Maybe it should hold some of its own, it could be 
		// interesting to try a SequencerComponent as well though, with their own triggers.

		// NR: SelectedSequencerComponent on entity level?
		std::vector<SSequencerEntityTrack> Tracks;
		SSequencerContextData Data;
		U32 InternalCurrentFrame = 0;
		F32 TickTime = 0.0f;
	};
}