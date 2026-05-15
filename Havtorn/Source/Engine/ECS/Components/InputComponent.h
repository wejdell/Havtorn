// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Assets/AssetReference.h"

namespace Havtorn
{
	struct SInputComponent : public SComponent
	{
		SInputComponent() = default;
		SInputComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}
		
		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);

		void IsDeleted(CScene* fromScene) override;

		SAssetReference AssetReference;
	};
}
