// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Graphics/GraphicsStructs.h"
#include "FileSystem/FileHeaderDeclarations.h"

namespace Havtorn
{
	struct SSkeletalMeshComponent : public SComponent
	{
		SSkeletalMeshComponent() = default;
		SSkeletalMeshComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}
		SSkeletalMeshComponent(const SEntity& entityOwner, const std::string& assetPath)
			: SComponent(entityOwner)
			, AssetReference(SAssetReference(assetPath))
		{}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		SAssetReference AssetReference;
	};
}
