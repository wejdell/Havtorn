// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DecalComponentView.h"

#include "ECS/Components/DecalComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"
#include "Graphics/TextureBank.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
	SComponentViewResult SDecalComponentView::View(const SEntity& entityOwner, CScene* scene)
	{
		SDecalComponent* decalComp = scene->GetComponent<SDecalComponent>(entityOwner);

		ImGui::Checkbox("Render Albedo", &decalComp->ShouldRenderAlbedo);
		ImGui::Checkbox("Render Material", &decalComp->ShouldRenderMaterial);
		ImGui::Checkbox("Render Normal", &decalComp->ShouldRenderNormal);

		SComponentViewResult result;
		for (Havtorn::U8 materialIndex = 0; materialIndex < decalComp->TextureReferences.size(); materialIndex++)
		{
			if (materialIndex % 3 == 0)
				ImGui::Text("Albedo");

			if (materialIndex % 3 == 1)
				ImGui::Text("Material");

			if (materialIndex % 3 == 2)
				ImGui::Text("Normal");

			Havtorn::U16 ref = decalComp->TextureReferences[materialIndex];
			if (ImGui::ImageButton((void*)GEngine::GetTextureBank()->GetTexture(ref), ImGui::UUtils::TexturePreviewSize))
			{
				result.Label = EComponentViewResultLabel::OpenTextureAssetModal;
				result.ComponentSubIndex = Havtorn::UMath::Min(materialIndex, static_cast<Havtorn::U8>(decalComp->TextureReferences.size() - 1));
				result.ComponentViewed = decalComp;

				ImGui::OpenPopup(ImGui::UUtils::SelectTextureModalName.c_str());
				ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImGui::UUtils::ModalWindowPivot);
			}
		}

		//MaterialRefToChangeIndex = Havtorn::UMath::Min(MaterialRefToChangeIndex, static_cast<Havtorn::U16>(decalComp.TextureReferences.size() - 1));
		//OpenSelectTextureAssetModal(decalComp.TextureReferences[MaterialRefToChangeIndex]);

		return result;
	}
}
