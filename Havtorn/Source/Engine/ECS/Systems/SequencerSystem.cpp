// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "SequencerSystem.h"
#include "Scene/Scene.h"
#include "SequencerKeyframes/SequencerKeyframe.h"
#include "ECS/Components/SequencerComponent.h"
#include "Assets/SequencerAsset.h"

#include "SequencerKeyframes/SequencerTransformKeyframe.h"
#include "SequencerKeyframes/SequencerSpriteKeyframe.h"
#include "ECS/Components/TransformComponent.h"

#include "Core/HavtornString.h"
#include "Engine.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileHeaderDeclarations.h"

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

		//std::vector<SSequencerComponent>& sequencerComponents = scene->GetSequencerComponents();

		//if (ShouldRecordNewKeyframes)
		//	RecordNewKeyframes(scene, sequencerComponents);

		Tick(scene);

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

		SequencerAssets.emplace_back(new CSequencerAsset());
		CurrentSequencer = SequencerAssets[0];
		CurrentSequencer->Name = std::string("Intro");

		std::vector<SEntity>& entities = scene->GetEntities();

		U64 spriteGUID = entities[4].GUID;
		//AddComponentTrackToEntityReference(spriteGUID, EComponentType::SpriteComponent);
		//Data.CurrentFrame = 20;
		//AddEmptyKeyframeToComponent<SSequencerSpriteKeyframe>(spriteGUID, EComponentType::SpriteComponent);
		//Data.CurrentFrame = 24;
		//AddEmptyKeyframeToComponent<SSequencerSpriteKeyframe>(spriteGUID, EComponentType::SpriteComponent);
		//Data.CurrentFrame = 30;
		//auto newKeyframe = AddEmptyKeyframeToComponent<SSequencerSpriteKeyframe>(spriteGUID, EComponentType::SpriteComponent);
		//newKeyframe->ShouldBlendRight = false;
		//newKeyframe->ShouldBlendLeft = false;
		//Data.CurrentFrame = 40;
		//newKeyframe = AddEmptyKeyframeToComponent<SSequencerSpriteKeyframe>(spriteGUID, EComponentType::SpriteComponent);
		//newKeyframe->ShouldBlendLeft = false;
		//Data.CurrentFrame = 60;
		//newKeyframe = AddEmptyKeyframeToComponent<SSequencerSpriteKeyframe>(spriteGUID, EComponentType::SpriteComponent);
		//newKeyframe->ShouldBlendRight = false;

		AddComponentTrackToEntityReference(spriteGUID, EComponentType::TransformComponent);
		Data.CurrentFrame = 18;
		AddEmptyKeyframeToComponent<SSequencerTransformKeyframe>(spriteGUID, EComponentType::TransformComponent);
		Data.CurrentFrame = 29;
		AddEmptyKeyframeToComponent<SSequencerTransformKeyframe>(spriteGUID, EComponentType::TransformComponent);
		
		Data.CurrentFrame = 0;

		//LoadSequencerFromFile(std::string("Assets/Sequencers/Intro.hva"));
	}

	void CSequencerSystem::SetSequencerContextData(const SSequencerContextData& data)
	{
		Data = data;
	}

	SSequencerContextData CSequencerSystem::GetSequencerContextData() const
	{
		return Data;
	}

	void CSequencerSystem::AddComponentTrackToEntityReference(const U64 guid, EComponentType trackComponentType)
	{
		if (!CurrentSequencer)
			return;

		SSequencerEntityReference* entityReferencePointer = CurrentSequencer->TryGetEntityReference(guid);
		if (!entityReferencePointer)
		{
			CurrentSequencer->EntityReferences.push_back(guid);
		}

		entityReferencePointer = CurrentSequencer->TryGetEntityReference(guid);
		if (!entityReferencePointer)
			return;

		entityReferencePointer->ComponentTracks.push_back(SSequencerComponentTrack{ trackComponentType });
	}

	const EComponentType CSequencerSystem::GetComponentTrackTypeFromEntityReference(const U64 guid, const I32 componentTrackIndex)
	{
		if (!CurrentSequencer)
			return EComponentType::Count;

		SSequencerEntityReference* const entityReferencePointer = CurrentSequencer->TryGetEntityReference(guid);
		if (!entityReferencePointer)
			return EComponentType::Count;

		if (!UMath::IsWithin(componentTrackIndex, I32(0), static_cast<I32>(entityReferencePointer->ComponentTracks.size())))
			return EComponentType::Count;

		return entityReferencePointer->ComponentTracks[componentTrackIndex].ComponentType;
	}

	SSequencerKeyframe* const CSequencerSystem::GetKeyframeFromEntityReference(const U64 guid, const I32 componentTrackIndex, const I32 keyframeIndex)
	{
		if (!CurrentSequencer)
			return nullptr;

		SSequencerEntityReference* const entityReferencePointer = CurrentSequencer->TryGetEntityReference(guid);
		if (!entityReferencePointer)
			return nullptr;

		if (!UMath::IsWithin(componentTrackIndex, I32(0), static_cast<I32>(entityReferencePointer->ComponentTracks.size())))
			return nullptr;

		if (!UMath::IsWithin(keyframeIndex, I32(0), static_cast<I32>(entityReferencePointer->ComponentTracks[componentTrackIndex].Keyframes.size())))
			return nullptr;

		return entityReferencePointer->ComponentTracks[componentTrackIndex].Keyframes[keyframeIndex];
	}

	void CSequencerSystem::RecordNewKeyframes(CScene* scene)
	{
		if (!CurrentSequencer)
			return;

		ShouldRecordNewKeyframes = false;

		for (SSequencerEntityReference& entityReference : CurrentSequencer->EntityReferences)
		{
			for (SSequencerComponentTrack& componentTrack : entityReference.ComponentTracks)
			{
				U16 numberOfKeyframes = static_cast<U16>(componentTrack.Keyframes.size());
				for (U16 index = 0; index < numberOfKeyframes; index++)
				{
					SSequencerKeyframe* keyframe = componentTrack.Keyframes[index];
					if (!keyframe->ShouldRecord)
						continue;

					keyframe->ShouldRecord = false;
					keyframe->SetEntityDataOnKeyframe(scene, scene->GetSceneIndex(entityReference.GUID));
				}
			}
		}
	}

	void CSequencerSystem::SortKeyframes(const U64 guid, U32 trackIndex, I32& lastEditedKeyframeIndex)
	{
		SSequencerEntityReference* const entityReferencePointer = CurrentSequencer->TryGetEntityReference(guid);
		if (!entityReferencePointer)
			return;

		if (!UMath::IsWithin(trackIndex, (U32)0, static_cast<U32>(entityReferencePointer->ComponentTracks.size())))
			return;

		SSequencerComponentTrack& componentTrack = entityReferencePointer->ComponentTracks[trackIndex];
		if (componentTrack.Keyframes.size() < 2)
			return;

		std::sort(componentTrack.Keyframes.begin(), componentTrack.Keyframes.end(), [](SSequencerKeyframe* a, SSequencerKeyframe* b) { return a->FrameNumber < b->FrameNumber; });

		U16 numberOfKeyframes = static_cast<U16>(componentTrack.Keyframes.size());
		U32 lastFrameNumber = componentTrack.Keyframes[0]->FrameNumber;
		for (U16 index = 1; index < numberOfKeyframes; index++)
		{
			SSequencerKeyframe* keyframe = componentTrack.Keyframes[index];

			// NR: Frame number collision handling for edited keyframe
			if (index == lastEditedKeyframeIndex && keyframe->FrameNumber == lastFrameNumber)
			{
				std::swap(componentTrack.Keyframes[index], componentTrack.Keyframes[index - 1]);
				componentTrack.Keyframes[index]->FrameNumber++;
				lastEditedKeyframeIndex--;
				continue;
			}
			else if (index == lastEditedKeyframeIndex + 1 && keyframe->FrameNumber == lastFrameNumber)
			{
				std::swap(componentTrack.Keyframes[index], componentTrack.Keyframes[index - 1]);
				componentTrack.Keyframes[index - 1]->FrameNumber--;
				lastEditedKeyframeIndex++;
				continue;
			}

			// NR: Allow keyframes to cascade if there are a number of them lined up.
			if (keyframe->FrameNumber == lastFrameNumber)
				keyframe->FrameNumber++;

			lastFrameNumber = keyframe->FrameNumber;
		}
	}

	const char* CSequencerSystem::GetCurrentSequencerName() const
	{
		if (CurrentSequencer == nullptr)
			return nullptr;

		return CurrentSequencer->Name.Data();
	}

	const std::vector<SSequencerEntityReference>* CSequencerSystem::GetCurrentEntityReferences() const
	{
		if (CurrentSequencer == nullptr)
			return nullptr;

		return &CurrentSequencer->EntityReferences;
	}

	void CSequencerSystem::SaveCurrentSequencer(const std::string& filePath)
	{
		if (CurrentSequencer == nullptr)
			return;

		SSequencerFileHeader fileHeader;
		fileHeader.SequencerName = CurrentSequencer->Name.AsString();
		fileHeader.SequencerNameLength = static_cast<U32>(fileHeader.SequencerName.size());
		fileHeader.NumberOfEntityReferences = static_cast<U32>(CurrentSequencer->EntityReferences.size());
		fileHeader.EntityReferences = CurrentSequencer->EntityReferences;

		const U32 fileSize = fileHeader.GetSize();
		char* data = new char[fileSize];

		fileHeader.Serialize(data);
		GEngine::GetFileSystem()->Serialize(filePath, data, fileSize);

		delete[] data;
	}

	void CSequencerSystem::LoadSequencerFromFile(const std::string& filePath)
	{
		U8 nameLength = static_cast<U8>(filePath.find_last_of(".") - (filePath.find_last_of("/") + 1));
		std::string sequencerName = filePath.substr(filePath.find_last_of("/") + 1, nameLength);

		if (CSequencerAsset* asset = TryGetSequencerWithName(sequencerName))
			return LoadSequencer(sequencerName);

		SSequencerFileHeader sequencerFile;

		const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(filePath);
		char* data = new char[fileSize];

		GEngine::GetFileSystem()->Deserialize(filePath, data, static_cast<U32>(fileSize));
		sequencerFile.Deserialize(data);

		CSequencerAsset* newAsset = SequencerAssets.emplace_back(new CSequencerAsset());
		newAsset->Name = sequencerFile.SequencerName;
		newAsset->EntityReferences = sequencerFile.EntityReferences;

		CurrentSequencer = newAsset;

		delete[] data;
	}

	void CSequencerSystem::LoadSequencer(const std::string& sequencerName)
	{
		if (CSequencerAsset* asset = TryGetSequencerWithName(sequencerName))
			CurrentSequencer = asset;
	}

	CSequencerAsset* CSequencerSystem::TryGetSequencerWithName(const std::string& sequencerName) const
	{
		for (CSequencerAsset* asset : SequencerAssets)
		{
			if (asset->Name == sequencerName)
				return asset;
		}

		return nullptr;
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
		for (SSequencerEntityReference& entityReference : CurrentSequencer->EntityReferences)
		{
			for (SSequencerComponentTrack& componentTrack : entityReference.ComponentTracks)
			{
				U16 numberOfKeyframes = static_cast<U16>(componentTrack.Keyframes.size());
				componentTrack.TrackState = ESequencerComponentTrackState::Waiting;

				// Start iterating from the start if we haven't yet or if we've reached the end
				if (componentTrack.CurrentKeyframeIndex == -1 || componentTrack.CurrentKeyframeIndex >= numberOfKeyframes - 1)
					componentTrack.CurrentKeyframeIndex = 0;

				for (U16 index = static_cast<U16>(componentTrack.CurrentKeyframeIndex); index < numberOfKeyframes; index++)
				{
					componentTrack.CurrentKeyframeIndex = index;
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

				componentTrack.CurrentKeyframe->SetKeyframeDataOnEntity(scene, scene->GetSceneIndex(entityReference.GUID));
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
