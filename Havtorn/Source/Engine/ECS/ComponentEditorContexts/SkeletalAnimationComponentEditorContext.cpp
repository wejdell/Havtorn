// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SkeletalAnimationComponentEditorContext.h"

#include "ECS/Components/SkeletalAnimationComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>
#include <Graphics/Debug/DebugDrawUtility.h>
#include <ECS/Components/TransformComponent.h>
#include <Assets/RuntimeAssetDeclarations.h>
#include "Assets/AssetRegistry.h"

namespace Havtorn
{
	SSkeletalAnimationComponentEditorContext SSkeletalAnimationComponentEditorContext::Context = {};

	SComponentViewResult Havtorn::SSkeletalAnimationComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!GUI::TryOpenComponentView("SkeletalAnimation"))
			return SComponentViewResult();

		SSkeletalAnimationComponent* skeletalAnimationComp = scene->GetComponent<SSkeletalAnimationComponent>(entityOwner);

		for (U32 index = 0; index < STATIC_U32(skeletalAnimationComp->AssetReferences.size()); index++)
		{
			GUI::PushID(STATIC_I32(index));
			SAssetReference& animRef = skeletalAnimationComp->AssetReferences[index];
			GUI::TextDisabled(UGeneralUtils::ExtractFileBaseNameFromPath(animRef.FilePath).c_str());
			GUI::SameLine();

			auto it = std::ranges::find(skeletalAnimationComp->PlayData, index, &SSkeletalAnimationPlayData::AssetReferenceIndex);
			const bool isPlaying = it != skeletalAnimationComp->PlayData.end();
			if (GUI::RadioButton("Play", isPlaying))
			{
				if (isPlaying)
					skeletalAnimationComp->PlayData.erase(it);
				else
					skeletalAnimationComp->PlayData.push_back(SSkeletalAnimationPlayData(index));
			}
			GUI::PopID();
		}

		GUI::Separator();
		GUI::TextDisabled("Playing Animations:");
		if (skeletalAnimationComp->PlayData.size() > 0)
		{
			GUI::TextDisabled(std::string("A: " + UGeneralUtils::ExtractFileBaseNameFromPath(skeletalAnimationComp->AssetReferences[skeletalAnimationComp->PlayData[0].AssetReferenceIndex].FilePath)).c_str());
			if (skeletalAnimationComp->PlayData.size() > 1)
			{
				GUI::TextDisabled(std::string("B: " + UGeneralUtils::ExtractFileBaseNameFromPath(skeletalAnimationComp->AssetReferences[skeletalAnimationComp->PlayData[1].AssetReferenceIndex].FilePath)).c_str());
				GUI::PushItemWidth(100.0f);
				GUI::SliderFloat("2D Blend A : B", skeletalAnimationComp->BlendValue, 0.0f, 1.0f);
				GUI::PopItemWidth();
			}
		}
		else
			GUI::TextDisabled("None");

		GUI::Separator();

		GUI::TextDisabled("Animations");

		GUI::SameLine();
		if (GUI::Button("Add"))
			skeletalAnimationComp->AssetReferences.push_back(SAssetReference());

		GUI::SameLine();
		if (GUI::Button("Clear"))
		{
			skeletalAnimationComp->AssetReferences.clear();
			skeletalAnimationComp->PlayData.clear();
		}

		//for (SMatrix& bone : skeletalAnimationComp->Bones)
		//{
		//	SMatrix parentTransform = scene->GetComponent<STransformComponent>(entityOwner)->Transform.GetMatrix();
		//	SMatrix worldTransform = bone /** parentTransform*/;
		//	GDebugDraw::AddAxis(worldTransform.GetTranslation(), worldTransform.GetEuler(), worldTransform.GetScale() * 0.5f);
		//}

		return { EComponentViewResultLabel::InspectAssetComponent, skeletalAnimationComp, SAssetReference::ConvertToPointers(skeletalAnimationComp->AssetReferences), EAssetType::Animation };
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
