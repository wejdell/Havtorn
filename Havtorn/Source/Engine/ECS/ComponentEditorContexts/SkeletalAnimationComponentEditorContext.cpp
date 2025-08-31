// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SkeletalAnimationComponentEditorContext.h"

#include "ECS/Components/SkeletalAnimationComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>
#include <Graphics/Debug/DebugDrawUtility.h>
#include <ECS/Components/TransformComponent.h>
#include <Core/RuntimeAssetDeclarations.h>
#include "Scene/AssetRegistry.h"

namespace Havtorn
{
	SSkeletalAnimationComponentEditorContext SSkeletalAnimationComponentEditorContext::Context = {};

	SComponentViewResult Havtorn::SSkeletalAnimationComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!GUI::TryOpenComponentView("SkeletalAnimation"))
			return SComponentViewResult();

		SSkeletalAnimationComponent* skeletalAnimationComp = scene->GetComponent<SSkeletalAnimationComponent>(entityOwner);
		//SVector2<I32> data = { STATIC_I32(skeletalAnimationComp->AnimationData.X), STATIC_I32(skeletalAnimationComp->AnimationData.Y) };
		// TODO.NW: Make separate clamp values for vector types
		//GUI::DragInt2("Animation Data", data, 1.0f, 0, skeletalAnimationComp->DurationInTicks - 1);
		//skeletalAnimationComp->AnimationData = { STATIC_U32(data.X), STATIC_U32(data.Y) };
		const SSkeletalAnimationAsset* assetData = GEngine::GetAssetRegistry()->RequestAssetData<SSkeletalAnimationAsset>(skeletalAnimationComp->AssetReferences[skeletalAnimationComp->CurrentAnimationIndex], entityOwner.GUID);
		GUI::DragFloat("Animation Time", skeletalAnimationComp->CurrentAnimationTime, 0.01f, 0.0f, assetData->DurationInTicks / STATIC_F32(assetData->TickRate));
		GUI::Checkbox("Is Playing", skeletalAnimationComp->IsPlaying);

		I32 animIndex = Havtorn::UMath::Max(0, static_cast<I32>(skeletalAnimationComp->CurrentAnimationIndex));
		I32 maxCount = STATIC_I32(skeletalAnimationComp->AssetReferences.size() - 1);
		GUI::SliderInt("AnimationClip", animIndex, 0, maxCount);
		skeletalAnimationComp->CurrentAnimationIndex = animIndex;

		GUI::SliderFloat("Blend", skeletalAnimationComp->BlendValue, 0.0f, 1.0f);

		//for (SMatrix& bone : skeletalAnimationComp->Bones)
		//{
		//	SMatrix parentTransform = scene->GetComponent<STransformComponent>(entityOwner)->Transform.GetMatrix();
		//	SMatrix worldTransform = bone /** parentTransform*/;
		//	GDebugDraw::AddAxis(worldTransform.GetTranslation(), worldTransform.GetEuler(), worldTransform.GetScale() * 0.5f);
		//}

		return { EComponentViewResultLabel::InspectAssetComponent, skeletalAnimationComp, nullptr, &skeletalAnimationComp->AssetReferences, EAssetType::Animation };
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
