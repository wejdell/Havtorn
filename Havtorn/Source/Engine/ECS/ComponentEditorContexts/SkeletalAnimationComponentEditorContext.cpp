// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SkeletalAnimationComponentEditorContext.h"

#include "ECS/Components/SkeletalAnimationComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>


namespace Havtorn
{
	SSkeletalAnimationComponentEditorContext SSkeletalAnimationComponentEditorContext::Context = {};

	SComponentViewResult Havtorn::SSkeletalAnimationComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!GUI::TryOpenComponentView("SkeletalAnimation"))
			return SComponentViewResult();

		SSkeletalAnimationComponent* skeletalAnimationComp = scene->GetComponent<SSkeletalAnimationComponent>(entityOwner);
		I32 data[2] = { STATIC_I32(skeletalAnimationComp->AnimationData.X), STATIC_I32(skeletalAnimationComp->AnimationData.Y) };
		GUI::DragInt2("Animation Data", data);
		skeletalAnimationComp->AnimationData = { STATIC_U32(data[0]), STATIC_U32(data[1]) };

		return SComponentViewResult();
	}

	bool SSkeletalAnimationComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Skeletal Animation Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SSkeletalAnimationComponent>(entity);
		scene->AddComponentEditorContext(entity, &SSkeletalAnimationComponentEditorContext::Context);
		return true;
	}

	bool SSkeletalAnimationComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##SkeletalAnim"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SSkeletalAnimationComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SSkeletalAnimationComponentEditorContext::Context);
		return true;
	}
}
