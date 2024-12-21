// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "SpriteAnimatorGraphComponent.h"

namespace Havtorn
{
	SSpriteAnimatorGraphNode& SSpriteAnimatorGraphComponent::SetRoot(const CHavtornStaticString<32>& name, const std::string& evaluatorClassAndFunctionName)
	{
		Graph = SSpriteAnimatorGraphNode();
		Graph.EvaluateFunctionMapKey = std::hash<std::string>{}(evaluatorClassAndFunctionName);
		Graph.Name = name;
		Graph.AnimationClipKey = -1;
		return Graph;
	}

	U32 SSpriteAnimatorGraphComponent::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(Owner);

		size += Graph.GetSize();

		size += GetDataSize(STATIC_U32(AnimationClips.size()));
		for (const SSpriteAnimationClip& animationClip : AnimationClips)
			size += animationClip.GetSize();

		return size;
	}

	void SSpriteAnimatorGraphComponent::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(Owner, toData, pointerPosition);

		Graph.Serialize(toData, pointerPosition);

		SerializeData(STATIC_U32(AnimationClips.size()), toData, pointerPosition);
		for (const SSpriteAnimationClip& animationClip : AnimationClips)
			animationClip.Serialize(toData, pointerPosition);
	}

	void SSpriteAnimatorGraphComponent::Deserialize(const char* fromData, U64& pointerPosition)
	{
		DeserializeData(Owner, fromData, pointerPosition);

		Graph.Deserialize(fromData, pointerPosition);

		U32 numberOfAnimationClips = 0;
		DeserializeData(numberOfAnimationClips, fromData, pointerPosition);
		AnimationClips.resize(numberOfAnimationClips);
		for (SSpriteAnimationClip& animationClip : AnimationClips)
			animationClip.Deserialize(fromData, pointerPosition);
	}
}