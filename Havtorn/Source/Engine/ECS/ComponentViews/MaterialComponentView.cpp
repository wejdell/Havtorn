// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "MaterialComponentView.h"
#include "ECS/Components/MaterialComponent.h"

namespace Havtorn
{
    SComponentViewResult Havtorn::SMaterialComponentView::View(const SEntity& /*entityOwner*/, CScene* /*scene*/)
    {
		//SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(entityOwner);

		//for (Havtorn::U8 materialIndex = 0; materialIndex < materialComp->Materials.size(); materialIndex++)
		//{
		//	Havtorn::SEditorAssetRepresentation* assetRep = Manager->GetAssetRepFromName(materialComp->Materials[materialIndex].Name).get();

		//	ImGui::Separator();

		//	if (assetRep->Name.size() > 0)
		//		ImGui::Text(assetRep->Name.c_str());
		//	else
		//		ImGui::Text("Empty Material");

		//	if (ImGui::ImageButton(assetRep->TextureRef, { TexturePreviewSize.X, TexturePreviewSize.Y }))
		//	{
		//		MaterialToChangeIndex = materialIndex;
		//		ImGui::OpenPopup("Select Material Asset");
		//		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		//	}
		//}

		//OpenSelectMaterialAssetModal(materialComp, MaterialToChangeIndex);

		return SComponentViewResult();
    }
}
