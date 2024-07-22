// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EnvironmentLightComponentView.h"

#include "ECS/Components/EnvironmentLightComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"
#include "Graphics/TextureBank.h"

#include <Core/imgui.h>
#include <Havtorn/Utilities.h>

namespace Havtorn
{
    SComponentViewResult SEnvironmentLightComponentView::View(const SEntity& entityOwner, CScene* scene)
    {
		SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(entityOwner);

		ImGui::Text("Ambient Static Cubemap");
	
		SComponentViewResult result;
		Havtorn::U16 ref = environmentLightComp->AmbientCubemapReference;
		if (ImGui::ImageButton((void*)Havtorn::GEngine::GetTextureBank()->GetTexture(ref), ImGui::UUtils::TexturePreviewSize))
		{
			result.Label = EComponentViewResultLabel::OpenTextureAssetModal;
			result.ComponentViewed = environmentLightComp;

			ImGui::OpenPopup(ImGui::UUtils::SelectTextureModalName.c_str());
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImGui::UUtils::ModalWindowPivot);
		}

        return result;
    }
}
