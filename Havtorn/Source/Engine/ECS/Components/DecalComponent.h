// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Assets/AssetReference.h"

namespace Havtorn
{
	struct SDecalComponent : public SComponent
	{
		SDecalComponent() = default;
		SDecalComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}
		SDecalComponent(const SEntity& entityOwner, const std::string& assetPath)
			: SComponent(entityOwner)
			, AssetReference(assetPath)
		{}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		SAssetReference AssetReference;
	};
}
