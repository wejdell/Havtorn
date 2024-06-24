// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "SpriteAnimatorGraphNode.h"

namespace Havtorn
{
	struct SSpriteAnimatorGraphComponent : public SComponent
	{
		SSpriteAnimatorGraphComponent()
			: SComponent(EComponentType::SpriteAnimatorGraphComponent)
		{
		}

		HAVTORN_API SSpriteAnimatorGraphNode& SetRoot(const CHavtornStaticString<32>& name, std::function<I16(CScene*, U64)> evaluator);

		SSpriteAnimatorGraphNode Graph;
		U32 CurrentFrame = 0;
		I16 CurrentAnimationClipKey = 0;
		I16 ResolvedAnimationClipKey = 0;
		F32 ElapsedTimeInSeconds = 0.0f;

		// AS: Dont add animationClips directly to this vector. They are added using SSpriteAnimatorGraphNode::AddClipNode
		std::vector<SSpriteAnimationClip> AnimationClips;
	};
}