// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/Component.h"
#include "Assets/AssetReference.h"
#include "Assets/AssetFileHeader.h"

namespace Havtorn
{
	enum class ELevelLoadState
	{
		Unloaded,
		Unloading,
		Loading,
		Loaded
	};

	struct SSceneState
	{
		SAssetReference SceneReference = {};
		ELevelLoadState State = ELevelLoadState::Unloaded;
		CScene* ScenePointer = nullptr;
	};

	struct SLevelStreamingComponent : public SComponent
	{
		SLevelStreamingComponent() = default;
		SLevelStreamingComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{
		}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		std::vector<SSceneState> SceneStates;
		ELevelLoadState ComponentLoadState = ELevelLoadState::Unloaded;
	};
}
