// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteAnimatorGraphNode.h"
#include "ECS/Components/SpriteAnimatorGraphComponent.h"

namespace Havtorn
{
	U32 SSpriteAnimationClip::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(IsLooping);
		size += GetDataSize(UVRects);
		size += GetDataSize(Durations);
		
		return size;
	}

	void SSpriteAnimationClip::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(IsLooping, toData, pointerPosition);
		SerializeData(UVRects, toData, pointerPosition);
		SerializeData(Durations, toData, pointerPosition);
	}

	void SSpriteAnimationClip::Deserialize(const char* fromData, U64& pointerPosition)
	{
		DeserializeData(IsLooping, fromData, pointerPosition);
		DeserializeData(UVRects, fromData, pointerPosition);
		DeserializeData(Durations, fromData, pointerPosition);
	}

	SSpriteAnimatorGraphNode& SSpriteAnimatorGraphNode::AddSwitchNode(const CHavtornStaticString<32>& name, const std::string& evaluatorClassAndFunctionName)
	{
		SSpriteAnimatorGraphNode& switchNode = Nodes.emplace_back();
		switchNode.Name = name;
		switchNode.EvaluateFunctionMapKey = std::hash<std::string>{}(evaluatorClassAndFunctionName);
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

	U32 SSpriteAnimatorGraphNode::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(Name);
		
		size += GetDataSize(STATIC_U32(Nodes.size()));
		for (const SSpriteAnimatorGraphNode& node : Nodes)
			size += node.GetSize();
		
		size += GetDataSize(EvaluateFunctionMapKey);
		size += GetDataSize(AnimationClipKey);

		return size;
	}

	void SSpriteAnimatorGraphNode::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(Name, toData, pointerPosition);

		SerializeData(STATIC_U32(Nodes.size()), toData, pointerPosition);
		for (const SSpriteAnimatorGraphNode& node : Nodes)
			node.Serialize(toData, pointerPosition);

		SerializeData(EvaluateFunctionMapKey, toData, pointerPosition);
		SerializeData(AnimationClipKey, toData, pointerPosition);
	}

	void SSpriteAnimatorGraphNode::Deserialize(const char* fromData, U64& pointerPosition)
	{
		DeserializeData(Name, fromData, pointerPosition);

		U32 numberOfNodes = 0;
		DeserializeData(numberOfNodes, fromData, pointerPosition);
		Nodes.resize(numberOfNodes);
		for (SSpriteAnimatorGraphNode& node : Nodes)
		//for (U64 index = 0; index < numberOfNodes; index++)
			node.Deserialize(fromData, pointerPosition);

		DeserializeData(EvaluateFunctionMapKey, fromData, pointerPosition);
		DeserializeData(AnimationClipKey, fromData, pointerPosition);
	}
}