// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Graphics/GraphicsStructs.h"
#include <HavtornString.h>

namespace Havtorn
{
	struct SSkeletalAnimationComponent : public SComponent
	{
		SSkeletalAnimationComponent() = default;
		SSkeletalAnimationComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		ENGINE_API [[nodiscard]] U32 GetSize() const;
		ENGINE_API void Serialize(char* toData, U64& pointerPosition) const;
		ENGINE_API void Deserialize(const char* fromData, U64& pointerPosition);

		// TODO.NR: This should be some of the only runtime data this component needs.
		//SVector2<U32> AnimationData; // X = animationIndex, Y = frameIndex
		std::vector <SBoneAnimationClip> CurrentAnimation = {};
		U32 CurrentAnimationIndex = 0;

		std::vector<SMatrix> Bones = {};
		F32 CurrentAnimationTime = 0.0f;

		// TODO.NR: Support multiple animations/montages on one component
		// TODO.NR: Load animations and use registry to serialize component, same as with meshes. The below should be a runtime asset easily written to a texture when needed.
		std::string AssetName;
		U32 DurationInTicks = 0;
		U32 TickRate = 0;
		F32 BlendValue = 0.0f;
		bool IsPlaying = true;
	};
}
