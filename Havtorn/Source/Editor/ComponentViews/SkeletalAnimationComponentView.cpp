// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SkeletalAnimationComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"

#include <ECS/Components/SkeletalAnimationComponent.h>
#include <ECS/Components/TransformComponent.h>
#include <Assets/AssetRegistry.h>
#include <Scene/Scene.h>

#include <Graphics/Debug/DebugDrawUtility.h>

#include <GUI.h>

namespace Havtorn
{
	void Havtorn::SSkeletalAnimationComponentView::View(const SEntity& entityOwner, CScene* scene) const
	{
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

		// TODO.NW: Skeleton debug draw
		//for (SMatrix& bone : skeletalAnimationComp->Bones)
		//{
		//	SMatrix parentTransform = scene->GetComponent<STransformComponent>(entityOwner)->Transform.GetMatrix();
		//	SMatrix worldTransform = bone /** parentTransform*/;
		//	GDebugDraw::AddAxis(worldTransform.GetTranslation(), worldTransform.GetEuler(), worldTransform.GetScale() * 0.5f);
		//}

		CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();
		inspector->InspectAssetComponent(skeletalAnimationComp, EAssetType::SkeletalAnimation, SAssetReference::ConvertToPointers(skeletalAnimationComp->AssetReferences));
	}
}
