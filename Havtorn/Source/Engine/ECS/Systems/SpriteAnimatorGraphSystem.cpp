// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteAnimatorGraphSystem.h"
#include "ECS/Components/SpriteComponent.h"
#include "ECS/Components/SpriteAnimatorGraphComponent.h"
#include "Scene/Scene.h"

namespace Havtorn
{
	void CSpriteAnimatorGraphSystem::Update(CScene* scene)
	{
		const F32 deltaTime = GTime::Dt();
		const std::vector<SSpriteAnimatorGraphComponent*>& spriteAnimatorGraphComponents = scene->GetComponents<SSpriteAnimatorGraphComponent>();

		for (SSpriteAnimatorGraphComponent* component : spriteAnimatorGraphComponents)
		{
			if (!component)
				continue;

			if (component->AnimationClips.size() == 0)
				continue;

			if (component->Graph.EvaluateFunctionMapKey != 0)
			{
				SSpriteAnimatorGraphNode* currentNode = &component->Graph;
				while (currentNode != nullptr)
				{
					if (currentNode->AnimationClipKey != -1)
					{
						component->ResolvedAnimationClipKey = currentNode->AnimationClipKey;
						break;
					}
					
					I16 evaluatedNodeIndex = EvaluateFunctionMap[currentNode->EvaluateFunctionMapKey](scene, component->Owner);
					if (evaluatedNodeIndex >= 0 && evaluatedNodeIndex < currentNode->Nodes.size())
						currentNode = &currentNode->Nodes[evaluatedNodeIndex];			
				}
			}

			scene->GetComponent<SSpriteComponent>(component)->UVRect = TickAnimationClip(*component, deltaTime);
		}
	}

	void CSpriteAnimatorGraphSystem::BindEvaluateFunction(std::function<I16(CScene*, const SEntity&)>& function, const std::string& classAndFunctionName)
	{
		U64 id = std::hash<std::string>{}(classAndFunctionName);

		if (EvaluateFunctionMap.contains(id))
			EvaluateFunctionMap.erase(id);

		EvaluateFunctionMap.emplace(id, function);
	}

	SVector4 CSpriteAnimatorGraphSystem::TickAnimationClip(SSpriteAnimatorGraphComponent& data, const F32 deltaTime)
	{
		data.ElapsedTimeInSeconds += deltaTime;	

		// AS: By design, We wait for the end of the current frame before we allow changing clip

		U64 durationIndex = UMath::Min(data.CurrentFrame, STATIC_U32(data.AnimationClips[data.CurrentAnimationClipKey].Durations.size() - 1));
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