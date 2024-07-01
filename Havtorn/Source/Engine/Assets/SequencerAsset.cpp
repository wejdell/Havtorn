// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "ECS/Systems/SequencerSystem.h"
#include "SequencerKeyframes/SequencerTransformKeyframe.h"
#include "SequencerKeyframes/SequencerSpriteKeyframe.h"

namespace Havtorn
{
	SSequencerEntityReference* Havtorn::CSequencerAsset::TryGetEntityReference(const U64 guid)
	{
		auto entityReferenceIterator = std::find(EntityReferences.begin(), EntityReferences.end(), guid);
		if (entityReferenceIterator != EntityReferences.end())
			return &(*entityReferenceIterator);

		return nullptr;
	}

	bool CSequencerAsset::operator==(const CSequencerAsset& other) const
	{
		return Name == other.Name;
	}

	bool CSequencerAsset::operator==(const std::string& otherName) const
	{
		return Name == otherName;
	}

	SSequencerEntityReference::SSequencerEntityReference(const U64 guid)
		: GUID(guid)
		, ComponentTracks({})
	{}

	U32 SSequencerEntityReference::GetSize() const
	{
		U32 size = 0;
		size += sizeof(U64);
		size += sizeof(U32);
		
		for (const SSequencerComponentTrack& componentTrack : ComponentTracks)
			size += componentTrack.GetSize();
		
		return size;
	}

	void SSequencerEntityReference::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeSimple(GUID, toData, pointerPosition);
		NumberOfComponentTracks = static_cast<U32>(ComponentTracks.size()); // mutable
		SerializeSimple(NumberOfComponentTracks, toData, pointerPosition);

		for (const SSequencerComponentTrack& componentTrack : ComponentTracks)
			componentTrack.Serialize(toData, pointerPosition);
	}

	void SSequencerEntityReference::Deserialize(const char* fromData, U64& pointerPosition)
	{
		DeserializeSimple(GUID, fromData, pointerPosition);
		DeserializeSimple(NumberOfComponentTracks, fromData, pointerPosition);

		for (I64 index = 0; index < NumberOfComponentTracks; index++)
		{
			ComponentTracks.emplace_back(); 
			ComponentTracks.back().Deserialize(fromData, pointerPosition);
		}
	}

	SSequencerComponentTrack::SSequencerComponentTrack(EComponentType componentType)
		: ComponentType(componentType)
	{}

	SSequencerComponentTrack::~SSequencerComponentTrack()
	{
		for (U64 index = 0; index < Keyframes.size(); index++)
			delete Keyframes[index];

		Keyframes.clear();
	}

	U32 SSequencerComponentTrack::GetSize() const
	{
		U32 size = 0;
		size += sizeof(EComponentType);
		size += sizeof(U32);
		
		for (SSequencerKeyframe* keyframe : Keyframes)
			size += keyframe->GetSize();

		return size;
	}

	void SSequencerComponentTrack::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeSimple(ComponentType, toData, pointerPosition);
		NumberOfKeyframes = static_cast<U32>(Keyframes.size());
		SerializeSimple(NumberOfKeyframes, toData, pointerPosition);

		for (SSequencerKeyframe* keyframe : Keyframes)
			keyframe->Serialize(toData, pointerPosition);
	}

	void SSequencerComponentTrack::Deserialize(const char* fromData, U64& pointerPosition)
	{
		DeserializeSimple(ComponentType, fromData, pointerPosition);
		DeserializeSimple(NumberOfKeyframes, fromData, pointerPosition);

		Keyframes.resize(NumberOfKeyframes);

		// TODO.NR: Can we map enum to type in default C++?

		for (I64 index = 0; index < NumberOfKeyframes; index++)
		{
			switch (ComponentType)
			{
			case EComponentType::TransformComponent:
			{
				Keyframes[index] = new SSequencerTransformKeyframe();
				break;
			}
			case EComponentType::SpriteComponent:
			{
				Keyframes[index] = new SSequencerSpriteKeyframe();
				break;
			}
			}

			Keyframes[index]->Deserialize(fromData, pointerPosition);
		}
	}
}
