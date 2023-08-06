// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteAnimatorGraphSystem.h"
#include "Engine.h"
#include "ECS/Components/SpriteAnimatorGraphComponent.h"
#include "ECS/Components/SpriteComponent.h"
#include "Input/InputMapper.h"


namespace Havtorn
{
	CSpriteAnimatorGraphSystem::CSpriteAnimatorGraphSystem()
	{
		GEngine::GetInput()->GetAxisDelegate(Havtorn::EInputAxisEvent::Right).AddMember(this, &CSpriteAnimatorGraphSystem::HandleAxisInput);
	}

	CSpriteAnimatorGraphSystem::~CSpriteAnimatorGraphSystem()
	{
	}

	void CSpriteAnimatorGraphSystem::Update(CScene* scene)
	{
		std::vector<SSpriteAnimatorGraphComponent>& spriteAnimatorGraphComponents = scene->GetSpriteAnimatorGraphComponents();
		std::vector<SSpriteComponent>& spriteComponents = scene->GetSpriteComponents();
		const F32 deltaTime = GTime::Dt();
		for (U64 i = 0; i < spriteAnimatorGraphComponents.size(); i++)
		{
			SSpriteAnimatorGraphComponent& component = spriteAnimatorGraphComponents[i];
			if (!component.IsInUse)
				continue;

			component.State.HorizontalKey = static_cast<I8>(horizontal); //Some system that controls the Animators State 
			component.ResolvedAnimationClipKey = component.Graph.Evaluate(component.State); //Evaluate AnimationClip based on State

			spriteComponents[i].UVRect = TickAnimationClip(component, deltaTime);
		}

		ResetInput();
	}

	SVector4 CSpriteAnimatorGraphSystem::TickAnimationClip(SSpriteAnimatorGraphComponent& c, const F32 deltaTime)
	{
		F32 duration = c.AnimationClips[c.CurrentAnimationClipKey].Durations[c.CurrentFrame];
		c.Time += deltaTime;
		if (c.Time >= duration)
		{
			bool animationClipHasChanged = c.CurrentAnimationClipKey != c.ResolvedAnimationClipKey;
			if (animationClipHasChanged)
			{
				c.CurrentAnimationClipKey = c.ResolvedAnimationClipKey;
			}

			U32 frameCount = static_cast<U32>(c.AnimationClips[c.CurrentAnimationClipKey].Durations.size());
			c.CurrentFrame = (c.CurrentFrame + 1) % frameCount;

			if (animationClipHasChanged)
			{
				c.CurrentFrame = 0;
			}

			c.Time = 0.0f;
		}
		return c.AnimationClips[c.CurrentAnimationClipKey].UVRects[c.CurrentFrame];
	}

	void CSpriteAnimatorGraphSystem::HandleAxisInput(const SInputAxisPayload payload)
	{
		horizontal = payload.AxisValue;
	}
	void CSpriteAnimatorGraphSystem::ResetInput()
	{
		horizontal = 0.0f;
	}
}