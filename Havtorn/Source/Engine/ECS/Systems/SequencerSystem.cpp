// Copyright 2023 Team Havtorn. All Rights Reserved.
// 
// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "SequencerSystem.h"
#include "Scene/Scene.h"
#include "SequencerKeyframes/SequencerKeyframe.h"
#include "ECS/Components/SequencerComponent.h"

//#include "SequencerKeyframes/SequencerTransformKeyframe.h"
#include "SequencerKeyframes/SequencerSpriteKeyframe.h"
#include "ECS/Components/TransformComponent.h"

namespace Havtorn
{
	void CSequencerSystem::Update(CScene* scene)
	{
		// Keyframes. Keyframe components with properties that can be lerped. Per keyframe maybe need to determine blend in and out behavior
		// Adding keyframe can be done more than once, basic workflow should be to move around or change values on an entity in the editor to key the 
		// keyable properties
		// 
		// The sequencer needs to be on another empty frame to key it with the current properties, otherwise maybe overwrite?
		// An entity needs a sequencer component to be able to be used in sequencer?
		//
		// For camera/cinematic sequences, we could add a curveComponent/PathComponent whose 1D parameter can be keyed trivially, keying 0 and 1 will 
		// make whatever is connected to the PathComponent follow the path over the duration
		//
		// Simple design is great, setting down key frames can enable easy animation for anything. SpriteAnimationComponent can be keyed to have a clip
		// play when the key is reached. 
		// 
		// To change different things at different keyframes on the component you just need to add more keyframes, there's potentially a track per 
		// component type
		// 
		// Different properties on the components need to be handled differently, lerpable stuff needs to lerp based on the two keyframes one is within
		// If there is no end keyframe, all properties are just set. 
		// 
		// For every component that has a track and keyframes, it needs to update every frame, not only when a new frame line is reached.
		// 
		// Enable automatic keyframing with a mode, where when you let go of a modified component it keys itself if the corresponding track is also 
		// selected, and there's no keyframe on the selected frame
		// 
		// There can be a lot of work done on previewing in the sequencer, the duration of the animation can be visualized, audio duration as well.

		std::vector<SSequencerComponent>& sequencerComponents = scene->GetSequencerComponents();

		if (ShouldRecordNewKeyframes)
			RecordNewKeyframes(scene, sequencerComponents);

		Tick(scene, sequencerComponents);

		if (IsInitialized)
			return;

		IsInitialized = true;
		// NR: Test code, remove includes when no longer necessary

		//std::vector<SEntity>& entities = scene->GetEntities();
		//std::vector<STransformComponent>& transformComponents = scene->GetTransformComponents();
		//U64 mainCameraIndex = scene->GetMainCameraIndex();

		//U64 mainCameraEntityGUID = entities[mainCameraIndex].GUID;
		//
		//SSequencerEntityTrack& mainCameraTrack = Tracks.emplace_back(SSequencerEntityTrack{ mainCameraEntityGUID, {} });
		//SSequencerComponentTrack& mainCameraTransformTrack = mainCameraTrack.ComponentTracks.emplace_back(SSequencerComponentTrack{ EComponentType::TransformComponent, {}, {}, {} });

		//SSequencerTransformKeyframe* firstKeyframe = new SSequencerTransformKeyframe();
		//firstKeyframe->FrameNumber = 0;
		//firstKeyframe->KeyframedMatrix = transformComponents[mainCameraIndex].Transform.GetMatrix();

		//SMatrix finalMatrix;		
		//finalMatrix = transformComponents[mainCameraIndex].Transform.GetMatrix();
		//finalMatrix.SetRotation({ 30.0f, -75.0f, 0.0f });
		//finalMatrix.SetTranslation({ 3.0f, 2.0f, -1.0f });

		//SSequencerTransformKeyframe* lastKeyframe = new SSequencerTransformKeyframe();
		//lastKeyframe->FrameNumber = 60;
		//lastKeyframe->KeyframedMatrix = finalMatrix;

		//mainCameraTransformTrack.Keyframes.push_back(firstKeyframe);
		//mainCameraTransformTrack.Keyframes.push_back(lastKeyframe);

		std::vector<SEntity>& entities = scene->GetEntities();

		U64 spriteGUID = entities[4].GUID;
		U64 sceneIndex = scene->GetSceneIndex(spriteGUID);
		Data.CurrentFrame = 20;
		AddEmptyKeyframeToComponent<SSequencerSpriteKeyframe>(scene->GetSequencerComponents()[sceneIndex], EComponentType::SpriteComponent);
		Data.CurrentFrame = 24;
		AddEmptyKeyframeToComponent<SSequencerSpriteKeyframe>(scene->GetSequencerComponents()[sceneIndex], EComponentType::SpriteComponent);
		Data.CurrentFrame = 0;
		//spriteTrack.ComponentTracks.emplace_back(SSequencerComponentTrack{ EComponentType::SpriteComponent, {}, {}, {} });
	}

	void CSequencerSystem::SetSequencerContextData(const SSequencerContextData& data)
	{
		Data = data;
	}

	SSequencerContextData CSequencerSystem::GetSequencerContextData() const
	{
		return Data;
	}

	void CSequencerSystem::AddComponentTrackToComponent(SSequencerComponent& sequencerComponent, EComponentType trackComponentType)
	{
		sequencerComponent.ComponentTracks.push_back(SSequencerComponentTrack{ trackComponentType });
	}

	void CSequencerSystem::RecordNewKeyframes(CScene* scene, std::vector<SSequencerComponent>& sequencerComponents)
	{
		ShouldRecordNewKeyframes = false;

		for (U64 componentIndex = 0; componentIndex < sequencerComponents.size(); componentIndex++)
		{
			SSequencerComponent& sequencerComponent = sequencerComponents[componentIndex];
			if (!sequencerComponent.IsInUse)
				continue;

			for (SSequencerComponentTrack& componentTrack : sequencerComponent.ComponentTracks)
			{
				U16 numberOfKeyframes = static_cast<U16>(componentTrack.Keyframes.size());
				for (U16 index = 0; index < numberOfKeyframes; index++)
				{
					SSequencerKeyframe* keyframe = componentTrack.Keyframes[index];
					if (!keyframe->ShouldRecord)
						continue;

					keyframe->ShouldRecord = false;
					keyframe->SetEntityDataOnKeyframe(scene, componentIndex);
				}
			}
		}
	}

	void CSequencerSystem::Tick(CScene* scene, std::vector<SSequencerComponent>& sequencerComponents)
	{
		if (!Data.IsPlayingSequence && Data.CurrentFrame == InternalCurrentFrame)
			return;

		F32 reciprocalPlayRate = 1.0f / Data.PlayRate;

		TickTime += GTime::Dt();

		if (TickTime > reciprocalPlayRate)
		{
			TickTime -= reciprocalPlayRate;
			
			Data.CurrentFrame = Data.IsLooping ? (1 + Data.CurrentFrame) % (1 + Data.MaxFrames) : UMath::Min(1 + Data.CurrentFrame, Data.MaxFrames);
				
			// TODO.NR: Should ideally be called after track logic has been updated, but only once
			if (!Data.IsLooping && Data.CurrentFrame == Data.MaxFrames)
				OnSequenceFinished();
		}
		
		UpdateTracks(scene, sequencerComponents);
		InternalCurrentFrame = Data.CurrentFrame;
	}

	void CSequencerSystem::UpdateTracks(CScene* scene, std::vector<SSequencerComponent>& sequencerComponents)
	{
		for (U64 componentIndex = 0; componentIndex < sequencerComponents.size(); componentIndex++)
		{
			SSequencerComponent& sequencerComponent = sequencerComponents[componentIndex];
			if (!sequencerComponent.IsInUse)
				continue;

			for (SSequencerComponentTrack& componentTrack : sequencerComponent.ComponentTracks)
			{
				U16 numberOfKeyframes = static_cast<U16>(componentTrack.Keyframes.size());
				componentTrack.TrackState = ESequencerComponentTrackState::Waiting;

				// Start iterating from the start if we haven't yet or if we've have reached the end
				if (componentTrack.CurrentKeyframeIndex == -1 || componentTrack.CurrentKeyframeIndex >= numberOfKeyframes - 1)
					componentTrack.CurrentKeyframeIndex = 0;

				for (U16 index = static_cast<U16>(componentTrack.CurrentKeyframeIndex); index < numberOfKeyframes; index++)
				{
					componentTrack.CurrentKeyframeIndex = index;
					//SSequencerKeyframe& keyframe = *componentTrack.Keyframes[index];
					SSequencerKeyframe* keyframe = componentTrack.Keyframes[index];
					SSequencerKeyframe* nextKeyframe = (index + 1) < numberOfKeyframes ? componentTrack.Keyframes[index + 1] : nullptr;
					
					if (keyframe->FrameNumber <= Data.CurrentFrame)
					{
						componentTrack.CurrentKeyframe = keyframe;
						componentTrack.NextKeyframe = nextKeyframe;

						if (nextKeyframe != nullptr && nextKeyframe->FrameNumber > Data.CurrentFrame && keyframe->ShouldBlendRight && nextKeyframe->ShouldBlendLeft)
						{
							componentTrack.TrackState = ESequencerComponentTrackState::Blending;
							break;
						}

						componentTrack.TrackState = ESequencerComponentTrackState::Setting;
						break;
					}

					//if (keyframe.FrameNumber <= Data.CurrentFrame && (index + 1) < numberOfKeyframes && (*componentTrack.Keyframes[index + 1]).FrameNumber > Data.CurrentFrame)
					//{
					//	componentTrack.CurrentKeyframe = componentTrack.Keyframes[index];
					//	componentTrack.NextKeyframe = componentTrack.Keyframes[index + 1];
					//	componentTrack.TrackState = ESequencerComponentTrackState::Blending;
					//	break;
					//}
					//// Reached left side of last keyframe
					//else if (keyframe.FrameNumber <= Data.CurrentFrame && (index + 1) >= numberOfKeyframes) 
					//{
					//	componentTrack.CurrentKeyframe = componentTrack.Keyframes[index];
					//	componentTrack.NextKeyframe = nullptr;
					//	componentTrack.TrackState = ESequencerComponentTrackState::Setting;
					//	break;
					//}
				}

				// Don't set anything if we haven't reached a keyframe yet
				if (componentTrack.TrackState == ESequencerComponentTrackState::Waiting)
					break;

				// Only blend if both keyframes are set
				if (componentTrack.TrackState == ESequencerComponentTrackState::Blending)
				{
					F32 blendParam = UMath::Remap(static_cast<F32>(componentTrack.CurrentKeyframe->FrameNumber), static_cast<F32>(componentTrack.NextKeyframe->FrameNumber), 0.0f, 1.0f, static_cast<F32>(Data.CurrentFrame));
					componentTrack.CurrentKeyframe->Blend(componentTrack.NextKeyframe, blendParam);
				}
				else if (componentTrack.TrackState == ESequencerComponentTrackState::Setting)
				{
					componentTrack.CurrentKeyframe->Blend(nullptr, 0.0f);
				}

				componentTrack.CurrentKeyframe->SetKeyframeDataOnEntity(scene, componentIndex);
			}
		}
	}

	void CSequencerSystem::OnSequenceFinished()
	{
		// Event: Finished Sequence, Broadcast delegate
		Data.IsPlayingSequence = false;
		Data.CurrentFrame = 0;
		TickTime = 0.0f;
	}
}
