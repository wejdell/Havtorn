// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Graphics/GraphicsStructs.h"
#include "Assets/FileHeaderDeclarations.h"

namespace Havtorn
{
	struct SSkeletalAnimationPlayData
	{
		SSkeletalAnimationPlayData() = default;
		SSkeletalAnimationPlayData(U32 assetReferenceIndex)
			: AssetReferenceIndex(assetReferenceIndex)
		{}

		std::vector<SSkeletalPosedNode> LocalPosedNodes;
		U32 AssetReferenceIndex = 0;
		F32 CurrentAnimationTime = 0.0f;
	};

	struct SSkeletalAnimationComponent : public SComponent
	{
		SSkeletalAnimationComponent() = default;
		SSkeletalAnimationComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}
		SSkeletalAnimationComponent(const SEntity& entityOwner, const std::vector<std::string>& assetPaths)
			: SComponent(entityOwner)
			, AssetReferences(SAssetReference::MakeVectorFromPaths(assetPaths))
		{}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		std::vector<SAssetReference> AssetReferences;
		std::vector<SSkeletalAnimationPlayData> PlayData;

		std::vector<SMatrix> Bones = {};
		F32 BlendValue = 0.0f;
	};
}
