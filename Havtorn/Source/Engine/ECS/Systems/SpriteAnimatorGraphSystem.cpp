// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SpriteAnimatorGraphSystem.h"
#include "ECS/ComponentS/SpriteAnimatorGraphComponent.h"
#include "ECS/ComponentS/SpriteComponent.h"


namespace Havtorn
{
	CSpriteAnimatorGraphSystem::CSpriteAnimatorGraphSystem()
	{
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
			if (!spriteAnimatorGraphComponents[i].IsInUse)
				continue;

			spriteComponents[i].UVRect = TickAnimationClip(spriteAnimatorGraphComponents[i], deltaTime);
		}
	}

	//Gotta add This ComponentData to Inspector types in the Editor Stuffs..!

	SVector4 CSpriteAnimatorGraphSystem::TickAnimationClip(SSpriteAnimatorGraphComponent& c, const F32 deltaTime)
	{
		F32 duration = c.AnimationClips[c.CurrentAnimationClipKey].Durations[c.CurrentFrame];
		c.Time += deltaTime;
		if (c.Time >= duration)
		{
			U32 frameCount = static_cast<U32>(c.AnimationClips[c.CurrentAnimationClipKey].Durations.size());
			c.CurrentFrame = (c.CurrentFrame + 1) % frameCount;
			c.Time = 0.0f;
		}
		return c.AnimationClips[c.CurrentAnimationClipKey].UVRects[c.CurrentFrame];
	}
}