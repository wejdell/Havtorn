// Copyright 2023 Team Havtorn. All Rights Reserved.

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

	U32 Havtorn::SSequencerSpriteKeyframe::GetSize() const
	{
		U32 size = SSequencerKeyframe::GetSize();
		size += sizeof(SVector4);
		return size;
	}

	void Havtorn::SSequencerSpriteKeyframe::Serialize(char* toData, U64& pointerPosition) const
	{
		SSequencerKeyframe::Serialize(toData, pointerPosition);
		SerializeSimple(UVRect, toData, pointerPosition);
	}

	void Havtorn::SSequencerSpriteKeyframe::Deserialize(const char* fromData, U64& pointerPosition)
	{
		SSequencerKeyframe::Deserialize(fromData, pointerPosition);
		DeserializeSimple(UVRect, fromData, pointerPosition);
	}
}