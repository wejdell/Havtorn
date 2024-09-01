// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "InspectorWindow.h"

#include <ECS/ECSInclude.h>
#include <Engine.h>
#include <EditorManager.h>
#include <Graphics/RenderManager.h>
#include <Graphics/TextureBank.h>
#include <Scene/Scene.h>
#include <Core/GeneralUtilities.h>

#include "Windows/ViewportWindow.h"
#include "Windows/SpriteAnimatorGraphNodeWindow.h"

#include <Core/imgui.h>
#include <Core/ImGuizmo/ImGuizmo.h>
#include <Havtorn/Utilities.h>
#include "Utils/ImGuiUtils.h"

namespace ImGui
{
	using Havtorn::SEditorLayout;
	using Havtorn::SVector;
	using Havtorn::SVector4;
	using Havtorn::SVector2;
	using Havtorn::U64;
	using Havtorn::F32;

	CInspectorWindow::CInspectorWindow(const char* displayName, Havtorn::CEditorManager* manager)
		: CWindow(displayName, manager)
	{}

	CInspectorWindow::~CInspectorWindow()
	{
	}

	void CInspectorWindow::OnEnable()
	{
	}

	void CInspectorWindow::OnInspectorGUI()
	{
		const SEditorLayout& layout = Manager->GetEditorLayout();

		const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + layout.InspectorPosition.X, mainViewport->WorkPos.y + layout.InspectorPosition.Y));
		ImGui::SetNextWindowSize(ImVec2(layout.InspectorSize.X, layout.InspectorSize.Y));

		if (!ImGui::Begin(Name(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			ImGui::End();
			return;
		}

		Scene = Manager->GetCurrentScene();

		if (!Scene)
		{
			ImGui::End();
			return;
		}

		Havtorn::SEntity selection = Manager->GetSelectedEntity();
		if (!selection.IsValid())
		{
			ImGui::End();
			return;
		}

		SelectedEntity = selection;

		Havtorn::SMetaDataComponent* metaDataComp = Scene->GetComponent<Havtorn::SMetaDataComponent>(SelectedEntity);
		ImGui::HavtornInputText("", &metaDataComp->Name);
		ImGui::Separator();

		for (Havtorn::CScene::SViewFunctionPointer viewFunction : Scene->GetViews(SelectedEntity))
		{
			RemoveComponentButton();
			Havtorn::SComponentViewResult result = viewFunction(SelectedEntity, Scene);

			// TODO.NR: Could make this a enum-function map, but would be good to set up clear rules for how this should work.
			switch (result.Label)
			{
			case Havtorn::EComponentViewResultLabel::UpdateTransformGizmo:
				UpdateTransformGizmo(result);
				break;
			case Havtorn::EComponentViewResultLabel::InspectAssetComponent:
				InspectAssetComponent(result);
				break;
			case Havtorn::EComponentViewResultLabel::OpenSpriteAnimatorGraph:
				OpenAssetTool(result);
				break;
			case Havtorn::EComponentViewResultLabel::PassThrough:
			default:
				break;
			}

			ImGui::Dummy({ ImGui::UUtils::DummySizeX, ImGui::UUtils::DummySizeY });
		}

		if (ImGui::Button("Add Component", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
		{
			ImGui::OpenPopup("Add Component Modal");
		}

		OpenAddComponentModal();

		ImGui::End();
	}

	void CInspectorWindow::OnDisable()
	{
	}

	void CInspectorWindow::UpdateTransformGizmo(const Havtorn::SComponentViewResult& result)
	{
		if (Manager->GetIsFreeCamActive())
			return;

		Havtorn::STransformComponent* viewedTransformComponent = static_cast<Havtorn::STransformComponent*>(result.ComponentViewed);
		if (viewedTransformComponent == nullptr)
			return;

		CViewportWindow* viewportWindow = Manager->GetEditorWindow<CViewportWindow>();
		SVector2<F32> viewPortWindowDimensions = viewportWindow->GetRenderedSceneDimensions();
		SVector2<F32> viewPortWindowPosition = viewportWindow->GetRenderedScenePosition();

		ImGuizmo::SetDrawlist(viewportWindow->GetCurrentDrawList());
		ImGuizmo::SetRect(viewPortWindowPosition.X, viewPortWindowPosition.Y, viewPortWindowDimensions.X, viewPortWindowDimensions.Y);
		// TODO.NR: Figure out if we can use the clip rect
		//ImGui::PushClipRect({ viewPortWindowPosition.X, viewPortWindowPosition.Y }, { viewPortWindowPosition.X + viewPortWindowDimensions.X, viewPortWindowPosition.Y + viewPortWindowDimensions.Y }, true);

		Havtorn::SCameraComponent* cameraComp = Scene->GetComponent<Havtorn::SCameraComponent>(Scene->MainCameraEntity);
		Havtorn::STransformComponent* cameraTransformComp = Scene->GetComponent<Havtorn::STransformComponent>(Scene->MainCameraEntity);
		Havtorn::SMatrix inverseView = cameraTransformComp->Transform.GetMatrix().Inverse();

		Havtorn::SMatrix transformMatrix = viewedTransformComponent->Transform.GetMatrix();
		ImGuizmo::Manipulate(inverseView.data, cameraComp->ProjectionMatrix.data, static_cast<ImGuizmo::OPERATION>(Manager->GetCurrentGizmo()), ImGuizmo::LOCAL, transformMatrix.data);
		viewedTransformComponent->Transform.SetMatrix(transformMatrix);
		// TODO.NR: Figure out if we can use the clip rect
		//ImGui::PopClipRect();
	}

	void CInspectorWindow::InspectAssetComponent(const Havtorn::SComponentViewResult& result)
	{
		std::vector<std::string> assetNames = {};
		std::vector<std::string> assetLabels = {};
		std::string modalNameToOpen = "";

		if (Havtorn::SStaticMeshComponent* staticMeshComponent = dynamic_cast<Havtorn::SStaticMeshComponent*>(result.ComponentViewed))
		{
			assetNames.push_back(staticMeshComponent->Name.AsString());
			modalNameToOpen = SelectMeshAssetModalName;
		}
		else if (Havtorn::SMaterialComponent* materialComponent = dynamic_cast<Havtorn::SMaterialComponent*>(result.ComponentViewed))
		{
			for (auto& material : materialComponent->Materials)
				assetNames.push_back(material.Name);

			modalNameToOpen = SelectMaterialAssetModalName;
		}
		else if (Havtorn::SDecalComponent* decalComponent = dynamic_cast<Havtorn::SDecalComponent*>(result.ComponentViewed))
		{
			for (Havtorn::U16 textureRef : decalComponent->TextureReferences)
				assetNames.push_back(Havtorn::UGeneralUtils::ExtractFileNameFromPath(Havtorn::GEngine::GetTextureBank()->GetTexturePath(static_cast<Havtorn::U32>(textureRef))));

			modalNameToOpen = SelectTextureAssetModalName;
			assetLabels.push_back("Albedo");
			assetLabels.push_back("Material");
			assetLabels.push_back("Normal");
		}
		else if (Havtorn::SEnvironmentLightComponent* environmentLightComponent = dynamic_cast<Havtorn::SEnvironmentLightComponent*>(result.ComponentViewed))
		{
			assetNames.push_back(Havtorn::UGeneralUtils::ExtractFileNameFromPath(Havtorn::GEngine::GetTextureBank()->GetTexturePath(static_cast<Havtorn::U32>(environmentLightComponent->AmbientCubemapReference))));
			modalNameToOpen = SelectTextureAssetModalName;
		}
		else if (Havtorn::SSpriteComponent* spriteComponent = dynamic_cast<Havtorn::SSpriteComponent*>(result.ComponentViewed))
		{
			assetNames.push_back(Havtorn::UGeneralUtils::ExtractFileNameFromPath(Havtorn::GEngine::GetTextureBank()->GetTexturePath(static_cast<Havtorn::U32>(spriteComponent->TextureIndex))));
			modalNameToOpen = SelectTextureAssetModalName;
		}

		IterateAssetRepresentations(result, assetNames, assetLabels, modalNameToOpen);
	}

	void CInspectorWindow::IterateAssetRepresentations(const Havtorn::SComponentViewResult& result, const std::vector<std::string>& assetNames, const std::vector<std::string>& assetLabels, const std::string& modalNameToOpen)
	{
		for (Havtorn::U8 index = 0; index < static_cast<Havtorn::U8>(assetNames.size()); index++)
		{
			std::string assetName = assetNames[index];

			Havtorn::SEditorAssetRepresentation* assetRep = Manager->GetAssetRepFromName(assetName).get();

			ImGui::Separator();

			if (assetLabels.size() > index)
				ImGui::Text(assetLabels[index].c_str());
			else if (assetRep->Name.empty())
				ImGui::Text("N/A");
			else
				ImGui::Text(assetRep->Name.c_str());

			// TODO.NR: Update ImGui and use the overload that takes a unique identifier. This currently breaks when using two buttons with the same texture.
			if (ImGui::ImageButton(assetRep->TextureRef, { ImGui::UUtils::TexturePreviewSizeX, ImGui::UUtils::TexturePreviewSizeY }))
			{
				// TODO.NR: Make Algo lib function that finds index of object in array, make for-each loop
				AssetPickedIndex = index;
				ImGui::OpenPopup(modalNameToOpen.c_str());
				ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			}
		}

		if (modalNameToOpen == SelectMeshAssetModalName)
			OpenSelectMeshAssetModal(result);
		else if (modalNameToOpen == SelectMaterialAssetModalName)
			OpenSelectMaterialAssetModal(result);
		else if (modalNameToOpen == SelectTextureAssetModalName)
			OpenSelectTextureAssetModal(result);
	}

	void CInspectorWindow::OpenSelectMeshAssetModal(const Havtorn::SComponentViewResult& result)
	{
		if (!ImGui::BeginPopupModal("Select Mesh Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		Havtorn::SStaticMeshComponent* staticMesh = static_cast<Havtorn::SStaticMeshComponent*>(result.ComponentViewed);
		if (staticMesh == nullptr)
			return;

		F32 thumbnailPadding = 4.0f;
		F32 cellWidth = ImGui::UUtils::TexturePreviewSizeX * 0.75f + thumbnailPadding;
		F32 panelWidth = 256.0f;
		Havtorn::I32 columnCount = static_cast<Havtorn::I32>(panelWidth / cellWidth);
		Havtorn::U32 id = 0;

		if (!ImGui::BeginTable("NewMeshAssetTable", columnCount))
		{
			EndPopup();
			return;
		}

		/* TODO.NR: See if one can make a general folder structure exploration function,
		* return true and an assetRep if a directory is double clicked. Maybe multiple 
		* versions with slight variations. Want to be able to go to any directory. 
		* Alternative might be GetDoubleClickedAssetRep(startingDirectory), use return 
		* value as an optional.
		*/
		for (auto& entry : std::filesystem::recursive_directory_iterator("Assets/Tests"))
		{
			if (entry.is_directory())
				continue;

			auto& assetRep = Manager->GetAssetRepFromDirEntry(entry);

			ImGui::TableNextColumn();
			ImGui::PushID(id++);

			if (ImGui::ImageButton(assetRep->TextureRef, { ImGui::UUtils::TexturePreviewSizeX * 0.75f, ImGui::UUtils::TexturePreviewSizeY * 0.75f }))
			{
				Manager->GetRenderManager()->TryLoadStaticMeshComponent(assetRep->Name, staticMesh);

				Havtorn::SMaterialComponent* materialComp = Scene->GetComponent<Havtorn::SMaterialComponent>(staticMesh);
				if (materialComp != nullptr)
				{
					Havtorn::U8 meshMaterialNumber = staticMesh->NumberOfMaterials;
					Havtorn::I8 materialNumberDifference = meshMaterialNumber - static_cast<Havtorn::U8>(materialComp->Materials.size());

					// NR: Add materials to correspond with mesh
					if (materialNumberDifference > 0)
					{
						for (Havtorn::U8 i = 0; i < materialNumberDifference; i++)
						{
							materialComp->Materials.emplace_back();
						}
					}
					// NR: Remove materials to correspond with mesh
					else if (materialNumberDifference < 0)
					{
						for (Havtorn::U8 i = 0; i < materialNumberDifference * -1.0f; i++)
						{
							materialComp->Materials.pop_back();
						}
					}
					// NR: Do nothing
					else
					{ 
					}
				}

				ImGui::CloseCurrentPopup();
			}

			ImGui::Text(assetRep->Name.c_str());
			ImGui::PopID();
		}

		ImGui::EndTable();

		if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x, 0))) 
			ImGui::CloseCurrentPopup(); 

		ImGui::EndPopup();
	}

	void CInspectorWindow::OpenSelectTextureAssetModal(const Havtorn::SComponentViewResult& result)
	{
		if (!ImGui::BeginPopupModal("Select Texture Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			return;
		
		constexpr const char* searchPath = "Assets/Textures";

		if (Havtorn::SDecalComponent* decalComponent = dynamic_cast<Havtorn::SDecalComponent*>(result.ComponentViewed))
			HandleTextureAssetModal(searchPath, decalComponent->TextureReferences[result.ComponentSubIndex]);

		if (Havtorn::SEnvironmentLightComponent* environmentLightComponent = dynamic_cast<Havtorn::SEnvironmentLightComponent*>(result.ComponentViewed))
			HandleTextureAssetModal(searchPath, environmentLightComponent->AmbientCubemapReference);

		if (Havtorn::SSpriteComponent* spriteComponent = dynamic_cast<Havtorn::SSpriteComponent*>(result.ComponentViewed))
			HandleTextureAssetModal(searchPath, spriteComponent->TextureIndex);

		if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x, 0))) 
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	void CInspectorWindow::OpenSelectMaterialAssetModal(const Havtorn::SComponentViewResult& result)
	{
		if (!ImGui::BeginPopupModal("Select Material Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		Havtorn::SMaterialComponent* materialComponent = dynamic_cast<Havtorn::SMaterialComponent*>(result.ComponentViewed);
		if (materialComponent == nullptr)
			return;

		F32 thumbnailPadding = 4.0f;
		F32 cellWidth = ImGui::UUtils::TexturePreviewSizeX * 0.75f + thumbnailPadding;
		F32 panelWidth = 256.0f;
		Havtorn::I32 columnCount = static_cast<Havtorn::I32>(panelWidth / cellWidth);
		Havtorn::U32 id = 0;

		if (ImGui::BeginTable("NewMaterialAssetTable", columnCount))
		{
			for (auto& entry : std::filesystem::recursive_directory_iterator("Assets/Materials"))
			{
				if (entry.is_directory())
					continue;

				auto& assetRep = Manager->GetAssetRepFromDirEntry(entry);

				ImGui::TableNextColumn();
				ImGui::PushID(id++);

				if (ImGui::ImageButton(assetRep->TextureRef, { ImGui::UUtils::TexturePreviewSizeX * 0.75f, ImGui::UUtils::TexturePreviewSizeY * 0.75f }))
				{

					Manager->GetRenderManager()->TryReplaceMaterialOnComponent(assetRep->DirectoryEntry.path().string(), AssetPickedIndex, materialComponent);
					AssetPickedIndex = 0;
					ImGui::CloseCurrentPopup();
				}

				ImGui::Text(assetRep->Name.c_str());
				ImGui::PopID();
			}

			ImGui::EndTable();
		}

		if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x, 0))) { ImGui::CloseCurrentPopup(); }

		ImGui::EndPopup();	
	}

	void CInspectorWindow::OpenAssetTool(const Havtorn::SComponentViewResult& result)
	{
		Havtorn::SSpriteAnimatorGraphComponent* component = static_cast<Havtorn::SSpriteAnimatorGraphComponent*>(result.ComponentViewed);
		if (component == nullptr)
			return;

		Manager->GetEditorWindow<CSpriteAnimatorGraphNodeWindow>()->Inspect(*component);
	}

	void CInspectorWindow::HandleTextureAssetModal(const std::string& pathToSearch, Havtorn::U16& textureReference)
	{
		F32 cellWidth = ImGui::UUtils::TexturePreviewSizeX * 0.75f + ImGui::UUtils::ThumbnailPadding;
		Havtorn::I32 columnCount = static_cast<Havtorn::I32>(ImGui::UUtils::PanelWidth / cellWidth);
		Havtorn::U32 id = 0;

		if (ImGui::BeginTable("NewTextureAssetTable", columnCount))
		{
			for (auto& entry : std::filesystem::recursive_directory_iterator(pathToSearch.c_str()))
			{
				if (entry.is_directory())
					continue;

				auto& assetRep = Manager->GetAssetRepFromDirEntry(entry);

				ImGui::TableNextColumn();
				ImGui::PushID(id++);

				if (ImGui::ImageButton(assetRep->TextureRef, { ImGui::UUtils::TexturePreviewSizeX * 0.75f, ImGui::UUtils::TexturePreviewSizeY * 0.75f }))
				{
					textureReference = static_cast<Havtorn::U16>(Havtorn::GEngine::GetTextureBank()->GetTextureIndex(entry.path().string()));
					ImGui::CloseCurrentPopup();
				}

				ImGui::Text(assetRep->Name.c_str());
				ImGui::PopID();
			}

			ImGui::EndTable();
		}
	}

	void CInspectorWindow::OpenAddComponentModal()
	{
		if (!ImGui::BeginPopupModal("Add Component Modal", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		// TODO.NR: Support adding and removing components through the editor. Unsolved problem.
		//Havtorn::U32 id = 0;

		//if (ImGui::BeginTable("NewComponentTypeTable", 1))
		//{
		//	for (U64 i = 0; i < STATIC_U64(EComponentType::Count) - 2; i++)
		//	{
		//		ImGui::TableNextColumn();
		//		ImGui::PushID(id++);

		//		EComponentType componentType = static_cast<EComponentType>(i);
		//		if (ImGui::Button(GetComponentTypeString(componentType).c_str()))
		//		{
		//			Havtorn::CScene* scene = Manager->GetCurrentScene();
		//			Havtorn::SEntity* selection = Manager->GetSelectedEntity();

		//			if (scene != nullptr && selection != nullptr)
		//				scene->AddComponentToEntity(componentType, *selection);
		//			
		//			ImGui::CloseCurrentPopup();
		//		}

		//		ImGui::PopID();
		//	}

		//	ImGui::EndTable();
		//}

		//if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x, 0))) { ImGui::CloseCurrentPopup(); }

		ImGui::EndPopup();
	}
	
	void CInspectorWindow::RemoveComponentButton(/*Havtorn::EComponentType componentType*/)
	{
		// TODO.NR: Support adding and removing components through the editor. Unsolved problem.
		//ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - ImGui::GetTreeNodeToLabelSpacing()*0.3f);

		//ImGui::PushID(static_cast<U64>(componentType));
		//if (ImGui::Button("X"))
		//{
		//	Havtorn::CScene* scene = Manager->GetCurrentScene();
		//	Havtorn::SEntity* selection = Manager->GetSelectedEntity();

		//	if (scene == nullptr || selection == nullptr)
		//		return;

		//	scene->RemoveComponentFromEntity(componentType, *selection);
		//}
		//ImGui::PopID();
	}
}
