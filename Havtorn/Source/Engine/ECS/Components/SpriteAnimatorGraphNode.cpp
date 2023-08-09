// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteAnimatorGraphNode.h"
#include "ECS/Components/SpriteAnimatorGraphComponent.h"

namespace Havtorn
{
	SSpriteAnimatorGraphNode& SSpriteAnimatorGraphNode::AddSwitchNode(const CHavtornStaticString<32>& name, std::function<I16(CScene*, U64)> evaluator)
	{
		SSpriteAnimatorGraphNode& switchNode = Nodes.emplace_back();
		switchNode.Name = name;
		switchNode.Evaluate = evaluator;
		switchNode.AnimationClipKey = -1;
		return switchNode;
	}

	void SSpriteAnimatorGraphNode::AddClipNode(SSpriteAnimatorGraphComponent* component, const CHavtornStaticString<32>& name, const SSpriteAnimationClip& clipData)
	{
		SSpriteAnimatorGraphNode& clipNode = Nodes.emplace_back();
		clipNode.Name = name;
		clipNode.AnimationClipKey = static_cast<I16>(component->AnimationClips.size());
		component->AnimationClips.push_back(clipData);
	}
}