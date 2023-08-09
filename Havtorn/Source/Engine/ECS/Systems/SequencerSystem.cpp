// Copyright 2023 Team Havtorn. All Rights Reserved.
// 
// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "SequencerSystem.h"
#include "Scene/Scene.h"

namespace Havtorn
{
	void SSequencerTransformKeyframe::Blend(const SSequencerTransformKeyframe& nextKeyframe, F32 blendParam)
	{
		// NR: The decision to directly set versus blending should not be made inside 
		// the Blend function, but a level higher

		// TODO.NR: Try override virtual destructor in transformcomponent, see if inheritance solution works then

		IntermediateComponent = SMatrix::Interpolate(KeyframedComponent, nextKeyframe.KeyframedComponent, blendParam);
	}

	CSequencerSystem::CSequencerSystem()
	{
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

		Tick(scene);

		if (Tracks.size() > 0)
			return;

		std::vector<SEntity>& entities = scene->GetEntities();
		std::vector<STransformComponent>& transformComponents = scene->GetTransformComponents();
		U64 mainCameraIndex = scene->GetMainCameraIndex();

		U64 mainCameraEntityGUID = entities[mainCameraIndex].GUID;
		
		SSequencerEntityTrack& mainCameraTrack = Tracks.emplace_back(SSequencerEntityTrack{ mainCameraEntityGUID, {} });
		SSequencerComponentTrack& mainCameraTransformTrack = mainCameraTrack.ComponentTracks.emplace_back(SSequencerComponentTrack{ EComponentType::TransformComponent, {}, {}, {} });

		SSequencerTransformKeyframe firstKeyframe;
		firstKeyframe.FrameNumber = 0;
		//firstKeyframe.KeyframedComponent = transformComponents[mainCameraIndex];
		firstKeyframe.KeyframedComponent = transformComponents[mainCameraIndex].Transform.GetMatrix();

		SMatrix finalMatrix;		
		finalMatrix = transformComponents[mainCameraIndex].Transform.GetMatrix();
		finalMatrix.SetRotation({ 30.0f, -75.0f, 0.0f });
		finalMatrix.SetTranslation({ 3.0f, 2.0f, -1.0f });
		//STransformComponent finalTransformComponent;
		//finalTransformComponent.Transform.SetMatrix(finalMatrix);

		SSequencerTransformKeyframe lastKeyframe;
		lastKeyframe.FrameNumber = 60;
		//lastKeyframe.KeyframedComponent = finalTransformComponent;
		lastKeyframe.KeyframedComponent = finalMatrix;

		//mainCameraTransformTrack.Keyframes.push_back(new SSequencerTransformKeyframe{firstKeyframe});
		//mainCameraTransformTrack.Keyframes.push_back(new SSequencerTransformKeyframe{lastKeyframe});
		//*(mainCameraTransformTrack.Keyframes[0]) = firstKeyframe;
		//*(mainCameraTransformTrack.Keyframes[1]) = lastKeyframe;

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

				for (U16 index = 0; index < numberOfKeyframes; index++)
				{
					SSequencerTransformKeyframe keyframe = componentTrack.Keyframes[index];

					if (keyframe.FrameNumber <= Data.CurrentFrame && (index + 1) < numberOfKeyframes && componentTrack.Keyframes[index + 1].FrameNumber > Data.CurrentFrame)
					{
						componentTrack.CurrentKeyframe = componentTrack.Keyframes[index];
						componentTrack.NextKeyframe = componentTrack.Keyframes[index + 1];
						componentTrack.TrackState = ESequencerComponentTrackState::Blending;
						break;
					}
					else if (keyframe.FrameNumber <= Data.CurrentFrame && (index + 1) >= numberOfKeyframes) // Reached left side of last keyframe
					{
						componentTrack.CurrentKeyframe = componentTrack.Keyframes[index];
						componentTrack.NextKeyframe = componentTrack.Keyframes[index];
						componentTrack.TrackState = ESequencerComponentTrackState::Setting;
						break;
					}
				}

				// Don't set anything if we haven't reached a keyframe yet
				if (componentTrack.TrackState == ESequencerComponentTrackState::Waiting)
					break;

				// Set Result
				U64 sceneIndex = scene->GetSceneIndex(entityTrack.EntityGUID);

				switch (componentTrack.ComponentType)
				{
				case EComponentType::TransformComponent:
				{
					//SSequencerTransformKeyframe* transformKeyframe = reinterpret_cast<SSequencerTransformKeyframe*>(componentTrack.CurrentKeyframe);
					//SSequencerTransformKeyframe* nextTransformKeyframe = reinterpret_cast<SSequencerTransformKeyframe*>(componentTrack.NextKeyframe);

					SSequencerTransformKeyframe transformKeyframe = componentTrack.CurrentKeyframe;
					SSequencerTransformKeyframe nextTransformKeyframe = componentTrack.NextKeyframe;

					//if (transformKeyframe == nullptr || nextTransformKeyframe == nullptr)
					//	break;

					// Only blend if both keyframes are set
					if (componentTrack.TrackState == ESequencerComponentTrackState::Blending)
					{
						F32 blendParam = UMath::Remap(static_cast<F32>(transformKeyframe.FrameNumber), static_cast<F32>(nextTransformKeyframe.FrameNumber), 0.0f, 1.0f, static_cast<F32>(Data.CurrentFrame));
						//componentTrack.CurrentKeyframe.Blend(&componentTrack.NextKeyframe, blendParam);
						HV_LOG_DEBUG("Blend Param: %f", blendParam);
						transformKeyframe.Blend(nextTransformKeyframe, blendParam);
					}
					else if (componentTrack.TrackState == ESequencerComponentTrackState::Setting)
					{
						transformKeyframe.IntermediateComponent = transformKeyframe.KeyframedComponent;
					}

					STransformComponent& transformComponent = scene->GetTransformComponents()[sceneIndex];
					//transformComponent.Transform.SetMatrix(transformKeyframe.IntermediateComponent.Transform.GetMatrix());
					transformComponent.Transform.SetMatrix(transformKeyframe.IntermediateComponent);
				}
					break;
				default:
					break;
				}
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
