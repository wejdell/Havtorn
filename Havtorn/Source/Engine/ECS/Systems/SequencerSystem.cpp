// Copyright 2023 Team Havtorn. All Rights Reserved.
// 
// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "SequencerSystem.h"
#include "Scene/Scene.h"

namespace Havtorn
{
	void SSequencerTransformKeyframe::Blend(SSequencerKeyframe* nextKeyframe, F32 blendParam)
	{
		if (SSequencerTransformKeyframe* nextTransformKeyframe = dynamic_cast<SSequencerTransformKeyframe*>(nextKeyframe))
		{
			IntermediateMatrix = SMatrix::Interpolate(KeyframedMatrix, nextTransformKeyframe->KeyframedMatrix, blendParam);
			return;
		}

		IntermediateMatrix = KeyframedMatrix;
	}

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

		if (ShouldRecordNewKeyframes)
			RecordNewKeyframes(scene);

		Tick(scene);

		if (Tracks.size() > 0)
			return;

		std::vector<SEntity>& entities = scene->GetEntities();
		std::vector<STransformComponent>& transformComponents = scene->GetTransformComponents();
		U64 mainCameraIndex = scene->GetMainCameraIndex();

		U64 mainCameraEntityGUID = entities[mainCameraIndex].GUID;
		
		SSequencerEntityTrack& mainCameraTrack = Tracks.emplace_back(SSequencerEntityTrack{ mainCameraEntityGUID, {} });
		SSequencerComponentTrack& mainCameraTransformTrack = mainCameraTrack.ComponentTracks.emplace_back(SSequencerComponentTrack{ EComponentType::TransformComponent, {}, {}, {} });

		//SSequencerTransformKeyframe* keyframes = new SSequencerTransformKeyframe[13513];

		SSequencerTransformKeyframe* firstKeyframe = new SSequencerTransformKeyframe();
		firstKeyframe->FrameNumber = 0;
		firstKeyframe->KeyframedMatrix = transformComponents[mainCameraIndex].Transform.GetMatrix();

		SMatrix finalMatrix;		
		finalMatrix = transformComponents[mainCameraIndex].Transform.GetMatrix();
		finalMatrix.SetRotation({ 30.0f, -75.0f, 0.0f });
		finalMatrix.SetTranslation({ 3.0f, 2.0f, -1.0f });

		SSequencerTransformKeyframe* lastKeyframe = new SSequencerTransformKeyframe();
		lastKeyframe->FrameNumber = 60;
		lastKeyframe->KeyframedMatrix = finalMatrix;

		mainCameraTransformTrack.Keyframes.push_back(firstKeyframe);
		mainCameraTransformTrack.Keyframes.push_back(lastKeyframe);
	}

	void CSequencerSystem::SetSequencerContextData(const SSequencerContextData& data)
	{
		Data = data;
	}

	SSequencerContextData CSequencerSystem::GetSequencerContextData() const
	{
		return Data;
	}

	// NR: Don't want to couple this system more to specific keyframe implementations. Create recorded data outside this class
	void CSequencerSystem::RecordNewKeyframes(CScene* scene)
	{
		ShouldRecordNewKeyframes = false;

		for (SSequencerEntityTrack& entityTrack : Tracks)
		{
			for (SSequencerComponentTrack& componentTrack : entityTrack.ComponentTracks)
			{
				U16 numberOfKeyframes = static_cast<U16>(componentTrack.Keyframes.size());

				for (U16 index = 0; index < numberOfKeyframes; index++)
				{
					SSequencerKeyframe* keyframe = componentTrack.Keyframes[index];
					if (!keyframe->ShouldRecord)
						continue;

					keyframe->ShouldRecord = false;

					U64 sceneIndex = scene->GetSceneIndex(entityTrack.EntityGUID);

					switch (componentTrack.ComponentType)
					{
					case EComponentType::TransformComponent:
					{
						SSequencerTransformKeyframe* transformKeyframe = static_cast<SSequencerTransformKeyframe*>(keyframe);

						STransformComponent& transformComponent = scene->GetTransformComponents()[sceneIndex];
						transformKeyframe->KeyframedMatrix = transformComponent.Transform.GetMatrix();
					}
					break;

					default:
						break;
					}
				}
			}
		}
	}

	void CSequencerSystem::Tick(CScene* scene)
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
		
		UpdateTracks(scene);			
		InternalCurrentFrame = Data.CurrentFrame;
	}

	void CSequencerSystem::UpdateTracks(CScene* scene)
	{
		for (SSequencerEntityTrack& entityTrack : Tracks)
		{
			for (SSequencerComponentTrack& componentTrack : entityTrack.ComponentTracks)
			{
				U16 numberOfKeyframes = static_cast<U16>(componentTrack.Keyframes.size());
				componentTrack.TrackState = ESequencerComponentTrackState::Waiting;

				// Start iterating from the start if we haven't yet or if we've have reached the end
				if (componentTrack.CurrentKeyframeIndex == -1 || componentTrack.CurrentKeyframeIndex >= numberOfKeyframes - 1)
					componentTrack.CurrentKeyframeIndex = 0;

				for (U16 index = static_cast<U16>(componentTrack.CurrentKeyframeIndex); index < numberOfKeyframes; index++)
				{
					componentTrack.CurrentKeyframeIndex = index;
					SSequencerKeyframe& keyframe = *componentTrack.Keyframes[index];

					if (keyframe.FrameNumber <= Data.CurrentFrame && (index + 1) < numberOfKeyframes && (*componentTrack.Keyframes[index + 1]).FrameNumber > Data.CurrentFrame)
					{
						componentTrack.CurrentKeyframe = componentTrack.Keyframes[index];
						componentTrack.NextKeyframe = componentTrack.Keyframes[index + 1];
						componentTrack.TrackState = ESequencerComponentTrackState::Blending;
						break;
					}
					// Reached left side of last keyframe
					else if (keyframe.FrameNumber <= Data.CurrentFrame && (index + 1) >= numberOfKeyframes) 
					{
						componentTrack.CurrentKeyframe = componentTrack.Keyframes[index];
						componentTrack.NextKeyframe = nullptr;
						componentTrack.TrackState = ESequencerComponentTrackState::Setting;
						break;
					}
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

				SetKeyframeDataOnEntity(scene, entityTrack, componentTrack);
			}
		}
	}

	void CSequencerSystem::SetKeyframeDataOnEntity(CScene* scene, const SSequencerEntityTrack& entityTrack, const SSequencerComponentTrack& componentTrack)
	{
		// TODO.NR: This should perhaps be done once per frame instead of once per componentTrack, looping through the tracks 
		// again but only accessing the scene data once?

		U64 sceneIndex = scene->GetSceneIndex(entityTrack.EntityGUID);

		switch (componentTrack.ComponentType)
		{
			case EComponentType::TransformComponent:
			{
				SSequencerTransformKeyframe* transformKeyframe = static_cast<SSequencerTransformKeyframe*>(componentTrack.CurrentKeyframe);

				STransformComponent& transformComponent = scene->GetTransformComponents()[sceneIndex];
				transformComponent.Transform.SetMatrix(transformKeyframe->IntermediateMatrix);
			}
			break;

			default:
			break;
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
