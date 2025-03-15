// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "SpriteAnimatorGraphNode.h"

namespace Havtorn
{
	struct SSpriteAnimatorGraphComponent : public SComponent, public ISerializable
	{
		SSpriteAnimatorGraphComponent() = default;
		SSpriteAnimatorGraphComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{
		}

		ENGINE_API SSpriteAnimatorGraphNode& SetRoot(const CHavtornStaticString<32>& name, const std::string& evaluatorClassAndFunctionName);

		ENGINE_API [[nodiscard]] U32 GetSize() const;
		ENGINE_API void Serialize(char* toData, U64& pointerPosition) const;
		ENGINE_API void Deserialize(const char* fromData, U64& pointerPosition);

		SSpriteAnimatorGraphNode Graph;
		U32 CurrentFrame = 0;
		I16 CurrentAnimationClipKey = 0;
		I16 ResolvedAnimationClipKey = 0;
		F32 ElapsedTimeInSeconds = 0.0f;

		// AS: Dont add animationClips directly to this vector. They are added using SSpriteAnimatorGraphNode::AddClipNode
		std::vector<SSpriteAnimationClip> AnimationClips;
	};
}