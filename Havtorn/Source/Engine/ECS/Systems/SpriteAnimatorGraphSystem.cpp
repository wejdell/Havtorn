// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteAnimatorGraphSystem.h"
#include "ECS/Components/SpriteComponent.h"
#include "ECS/Components/SpriteAnimatorGraphComponent.h"

namespace Havtorn
{
	CSpriteAnimatorGraphSystem::CSpriteAnimatorGraphSystem()
	{}

	CSpriteAnimatorGraphSystem::~CSpriteAnimatorGraphSystem()
	{}

	void CSpriteAnimatorGraphSystem::Update(CScene* scene)
	{
		const F32 deltaTime = GTime::Dt();
		std::vector<SSpriteAnimatorGraphComponent>& spriteAnimatorGraphComponents = scene->GetSpriteAnimatorGraphComponents();
		std::vector<SSpriteComponent>& spriteComponents = scene->GetSpriteComponents();

		for (U64 i = 0; i < spriteAnimatorGraphComponents.size(); i++)
		{
			SSpriteAnimatorGraphComponent& component = spriteAnimatorGraphComponents[i];
			if (!component.IsInUse)
				continue;

			if(component.AnimationClips.size() == 0)
				continue;

			if (component.Graph.Evaluate)
			{
				SSpriteAnimatorGraphNode* currentNode = &component.Graph;
				while (currentNode != nullptr)
				{
					if (currentNode->AnimationClipKey != -1)
					{
						component.ResolvedAnimationClipKey = currentNode->AnimationClipKey;
						break;
					}
					
					I16 evaluatedNodeIndex = currentNode->Evaluate(scene, i);
					if (evaluatedNodeIndex >= 0 && evaluatedNodeIndex < currentNode->Nodes.size())
						currentNode = &currentNode->Nodes[evaluatedNodeIndex];			
				}
			}

			spriteComponents[i].UVRect = TickAnimationClip(component, deltaTime);
		}
	}

	SVector4 CSpriteAnimatorGraphSystem::TickAnimationClip(SSpriteAnimatorGraphComponent& data, const F32 deltaTime)
	{
		data.ElapsedTimeInSeconds += deltaTime;	

		// AS: By design, We wait for the end of the current frame before we allow changing clip

		U64 durationIndex = min(data.CurrentFrame, data.AnimationClips[data.CurrentAnimationClipKey].Durations.size() - 1);
		if (data.ElapsedTimeInSeconds >= data.AnimationClips[data.CurrentAnimationClipKey].Durations[durationIndex])
		{
			bool animationClipHasChanged = data.CurrentAnimationClipKey != data.ResolvedAnimationClipKey;
			if (animationClipHasChanged)
				data.CurrentAnimationClipKey = data.ResolvedAnimationClipKey;

			data.CurrentFrame = (data.CurrentFrame + 1) % data.AnimationClips[data.CurrentAnimationClipKey].KeyFrameCount();

			if (animationClipHasChanged)
				data.CurrentFrame = 0;

			data.ElapsedTimeInSeconds = 0.0f;
		}

		return data.AnimationClips[data.CurrentAnimationClipKey].UVRects[data.CurrentFrame];
	}
}