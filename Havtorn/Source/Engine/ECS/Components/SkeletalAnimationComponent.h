// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Graphics/GraphicsStructs.h"
#include "FileSystem/FileHeaderDeclarations.h"

namespace Havtorn
{
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

		//// TODO.NW: This should be some of the only runtime data this component needs.
		////SVector2<U32> AnimationData; // X = animationIndex, Y = frameIndex
		std::vector<SMatrix> Bones = {};
		U32 CurrentAnimationIndex = 0;
		F32 CurrentAnimationTime = 0.0f;
		F32 BlendValue = 0.0f;
		bool IsPlaying = true;
	};
}
