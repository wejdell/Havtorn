// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "StaticMeshComponentView.h"

#include "ECS/Components/StaticMeshComponent.h"

namespace Havtorn
{
    SComponentViewResult SStaticMeshComponentView::View(const SEntity& /*entityOwner*/, CScene* /*scene*/)
    {
		//SStaticMeshComponent* staticMesh = scene->GetComponent<SStaticMeshComponent>(entityOwner);

		//Havtorn::SEditorAssetRepresentation* assetRep = Manager->GetAssetRepFromName(staticMesh->Name.AsString()).get();

		//if (ImGui::ImageButton(assetRep->TextureRef, { TexturePreviewSize.X, TexturePreviewSize.Y }))
		//{
		//	ImGui::OpenPopup("Select Mesh Asset");
		//	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		//}
		//ImGui::Text(assetRep->Name.c_str());
		//ImGui::TextDisabled("Number Of Materials: %i", staticMesh->NumberOfMaterials);
		//OpenSelectMeshAssetModal(SelectedEntity);

        return SComponentViewResult();
    }
}
