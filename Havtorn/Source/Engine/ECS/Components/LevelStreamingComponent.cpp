// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "LevelStreamingComponent.h"
#include "Assets/AssetRegistry.h"

namespace Havtorn
{
	void SLevelStreamingComponent::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(Owner, toData, pointerPosition);

		SerializeData(static_cast<U32>(SceneStates.size()), toData, pointerPosition);
		for (auto& state : SceneStates)
		{
			state.SceneReference.Serialize(toData, pointerPosition);
		}
	}

	void SLevelStreamingComponent::Deserialize(const char* fromData, U64& pointerPosition)
	{
		DeserializeData(Owner, fromData, pointerPosition);

		U32 stateCount = 0;
		DeserializeData(stateCount, fromData, pointerPosition);
		for (U32 i = 0; i < stateCount; i++)
		{
			SSceneState& state = SceneStates.emplace_back(SSceneState());
			state.SceneReference.Deserialize(fromData, pointerPosition);
		}
	}

	U32 SLevelStreamingComponent::GetSize() const
	{
		U32 size = 0;

		size += GetDataSize(Owner);
		
		size += sizeof(U32);
		for (auto& state : SceneStates)
			size += state.SceneReference.GetSize();

		return size;
	}

	void SLevelStreamingComponent::IsDeleted(CScene* /*fromScene*/)
	{
		for (auto& state : SceneStates)
			GEngine::GetAssetRegistry()->UnrequestAsset(state.SceneReference, Owner.GUID);
	}
}
