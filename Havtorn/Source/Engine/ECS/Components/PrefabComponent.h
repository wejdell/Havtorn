// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Assets/AssetReference.h"

namespace Havtorn
{
	enum class EPrefabMode
	{
		Packed,
		Spawner
	};

	struct SPrefabComponent : public SComponent
	{
		SPrefabComponent() = default;
		SPrefabComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}
		SPrefabComponent(const SEntity& entityOwner, const std::string& assetPath)
			: SComponent(entityOwner)
			, AssetReference(SAssetReference(assetPath))
		{}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		SAssetReference AssetReference;
		EPrefabMode PrefabMode = EPrefabMode::Packed;
	};
}
