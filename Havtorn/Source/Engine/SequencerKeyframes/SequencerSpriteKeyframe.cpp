// Copyright 2023 Team Havtorn. All Rights Reserved.

// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "SequencerSpriteKeyframe.h"
#include "ECS/Components/SpriteComponent.h"
#include "Scene/Scene.h"

namespace Havtorn
{
	void SSequencerSpriteKeyframe::Blend(SSequencerKeyframe* /*nextKeyframe*/, F32 /*blendParam*/)
	{
	}

	void SSequencerSpriteKeyframe::SetEntityDataOnKeyframe(CScene* scene, U64 sceneIndex)
	{
		SSpriteComponent& spriteComponent = scene->GetSpriteComponents()[sceneIndex];
		UVRect = spriteComponent.UVRect;
	}

	void SSequencerSpriteKeyframe::SetKeyframeDataOnEntity(CScene* scene, U64 sceneIndex)
	{
		SSpriteComponent& spriteComponent = scene->GetSpriteComponents()[sceneIndex];
		spriteComponent.UVRect = UVRect;
	}
}