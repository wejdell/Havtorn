// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Assets/AssetReference.h"

namespace Havtorn
{
	struct SAudioEmitterComponent : public SComponent
	{
		SAudioEmitterComponent() = default;
		SAudioEmitterComponent(const SEntity& entityOwner);
		SAudioEmitterComponent(const SEntity& entityOwner, const std::vector<std::string>& assetPaths);

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		std::vector<SAssetReference> AssetReferences = { SAssetReference() };
		SVector LocalOffset = SVector::Zero;
		U64 AudioObjectID = 0;

	private:
		void Init();
	};
}
