// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "AnimatorGraphSystem.h"
#include "Graphics/RenderManager.h"
#include "ECS/Components/SkeletalAnimationComponent.h"
#include "ECS/Components/SkeletalMeshComponent.h"
#include "Scene/Scene.h"

namespace Havtorn
{
	CAnimatorGraphSystem::CAnimatorGraphSystem(CRenderManager* renderManager)
		: ISystem()
		, RenderManager(renderManager)
	{
	}

	void CAnimatorGraphSystem::Update(CScene* scene)
	{
		const F32 deltaTime = GTime::Dt();

		for (SSkeletalAnimationComponent* component : scene->GetComponents<SSkeletalAnimationComponent>())
		{
			if (!component->IsValid())
				continue;

			F32 frameDuration = 1.0f / STATIC_F32(component->TickRate);
			if ((component->CurrentFrameTime += deltaTime) >= frameDuration)
			{
				component->CurrentFrameTime -= frameDuration;
				component->AnimationData.Y = (component->AnimationData.Y + 1) % component->DurationInTicks;
			}
			
			RenderManager->WriteToAnimationDataTexture(component->AssetName);
		}
	}

	void CAnimatorGraphSystem::BindEvaluateFunction(std::function<I16(CScene*, const SEntity&)>& function, const std::string& classAndFunctionName)
	{
		U64 id = std::hash<std::string>{}(classAndFunctionName);

		if (EvaluateFunctionMap.contains(id))
			EvaluateFunctionMap.erase(id);

		EvaluateFunctionMap.emplace(id, function);
	}
}