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

#include <GUI.h>

namespace Havtorn
{
	CInspectorWindow::CInspectorWindow(const char* displayName, CEditorManager* manager)
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

		const SGuiViewport* mainViewport = GUI::GetMainViewport();

		//GUI::GUIProcess::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + layout.InspectorPosition.X, mainViewport->WorkPos.y + layout.InspectorPosition.Y));

		GUI::SetNextWindowPos(SVector2<F32>(mainViewport->WorkPos.x + layout.InspectorPosition.X, mainViewport->WorkPos.y + layout.InspectorPosition.Y));
		GUI::SetNextWindowSize(SVector2<F32>(layout.InspectorSize.X, layout.InspectorSize.Y));

		if (!GUI::Begin(Name(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			GUI::End();
			return;
		}

		Scene = Manager->GetCurrentScene();
		if (!Scene)
		{
			GUI::End();
			return;
		}

		SEntity selection = Manager->GetSelectedEntity();
		if (!selection.IsValid())
		{
			GUI::End();
			return;
		}

		SelectedEntity = selection;

		SMetaDataComponent* metaDataComp = Scene->GetComponent<SMetaDataComponent>(SelectedEntity);
		if (metaDataComp != nullptr)
		{
			GUI::HavtornInputText("##MetaDataCompName", &metaDataComp->Name);
			GUI::SameLine();
			GUI::TextDisabled("GUID %u", metaDataComp->Owner.GUID);
			if (GUI::IsItemHovered())
				GUI::SetTooltip("GUID %u", metaDataComp->Owner.GUID);
		}
		GUI::Separator();

		for (SComponentEditorContext* context : Scene->GetComponentEditorContexts(SelectedEntity))
		{
			if (context->RemoveComponent(SelectedEntity, Scene))
				continue;

			GUI::SameLine();
			SComponentViewResult result = context->View(SelectedEntity, Scene);

			// TODO.NR: Could make this a enum-function map, but would be good to set up clear rules for how this should work.
			switch (result.Label)
			{
			case EComponentViewResultLabel::UpdateTransformGizmo:
				UpdateTransformGizmo(result);
				break;
			case EComponentViewResultLabel::InspectAssetComponent:
				InspectAssetComponent(result);
				break;
			case EComponentViewResultLabel::OpenAssetTool:
				OpenAssetTool(result);
				break;
			case EComponentViewResultLabel::PassThrough:
			default:
				break;
			}

			GUI::Dummy({ GUI::DummySizeX, GUI::DummySizeY });
		}

		GUI::Separator();
		if (GUI::Button("Add Component", SVector2<F32>(GUI::GetContentRegionAvail().x, 0)))
			GUI::OpenPopup("Add Component Modal");

		OpenAddComponentModal();

		GUI::End();
	}

	void CInspectorWindow::OnDisable()
	{
	}

	void CInspectorWindow::UpdateTransformGizmo(const SComponentViewResult& result)
	{
		if (Manager->GetIsFreeCamActive())
			return;

		STransformComponent* viewedTransformComp = static_cast<STransformComponent*>(result.ComponentViewed);
		if (viewedTransformComp == nullptr)
			return;

		CViewportWindow* viewportWindow = Manager->GetEditorWindow<CViewportWindow>();
		SVector2<F32> viewportWindowDimensions = viewportWindow->GetRenderedSceneDimensions();
		SVector2<F32> viewportWindowPosition = viewportWindow->GetRenderedScenePosition();

		GUI::SetDrawlist(viewportWindow->GetCurrentDrawList());
		GUI::SetRect(viewportWindowPosition.X, viewportWindowPosition.Y, viewportWindowDimensions.X, viewportWindowDimensions.Y);
		
		SCameraComponent* cameraComp = Scene->GetComponent<SCameraComponent>(Scene->MainCameraEntity);
		STransformComponent* cameraTransformComp = Scene->GetComponent<STransformComponent>(Scene->MainCameraEntity);
		SMatrix viewMatrix = cameraTransformComp->Transform.GetMatrix();
		SMatrix inverseView = viewMatrix.Inverse();

		ViewManipulation(viewMatrix, viewportWindowPosition, viewportWindowDimensions);

		GUI::PushID(0);
		SMatrix transformMatrix = viewedTransformComp->Transform.GetMatrix();
		GUI::Manipulate(inverseView.data, cameraComp->ProjectionMatrix.data, Manager->GetCurrentGizmo(), GUI::LOCAL, transformMatrix.data, NULL, NULL);
		GUI::PopID();
		
		viewedTransformComp->Transform.SetMatrix(transformMatrix);
		cameraTransformComp->Transform.SetMatrix(viewMatrix);
	}

	void CInspectorWindow::ViewManipulation(SMatrix& outCameraView, const SVector2<F32>& windowPosition, const SVector2<F32>& windowSize)
	{
		const F32 viewManipulateRight = windowPosition.X + windowSize.X;
		const F32 viewManipulateTop = windowPosition.Y + 28.0f; // TODO.NR: Figure out all the menu bar sizes and stuff

		// TODO.NR: Focus on object, maybe make use of Focus function. Maybe add support for local and world View manipulation.

		const F32 camDistance = 1.0f;
		const F32 size = 128.0f;
		const U32 backgroundColor = 0x10101010;
		GUI::ViewManipulate(outCameraView.data, camDistance, SVector2<F32>(viewManipulateRight - size, viewManipulateTop), SVector2<F32>(size, size), backgroundColor);
	}

	void CInspectorWindow::InspectAssetComponent(const SComponentViewResult& result)
	{
		std::vector<std::string> assetNames = {};
		std::vector<std::string> assetLabels = {};
		std::string modalNameToOpen = "";

		if (SStaticMeshComponent* staticMeshComponent = dynamic_cast<SStaticMeshComponent*>(result.ComponentViewed))
		{
			assetNames.push_back(staticMeshComponent->Name.AsString());
			modalNameToOpen = SelectMeshAssetModalName;
		}
		else if (SMaterialComponent* materialComponent = dynamic_cast<SMaterialComponent*>(result.ComponentViewed))
		{
			for (auto& material : materialComponent->Materials)
				assetNames.push_back(material.Name);

			modalNameToOpen = SelectMaterialAssetModalName;
		}
		else if (SDecalComponent* decalComponent = dynamic_cast<SDecalComponent*>(result.ComponentViewed))
		{
			for (U16 textureRef : decalComponent->TextureReferences)
				assetNames.push_back(UGeneralUtils::ExtractFileBaseNameFromPath(GEngine::GetTextureBank()->GetTexturePath(static_cast<U32>(textureRef))));

			modalNameToOpen = SelectTextureAssetModalName;
			assetLabels.push_back("Albedo");
			assetLabels.push_back("Material");
			assetLabels.push_back("Normal");
		}
		else if (SEnvironmentLightComponent* environmentLightComponent = dynamic_cast<SEnvironmentLightComponent*>(result.ComponentViewed))
		{
			assetNames.push_back(UGeneralUtils::ExtractFileBaseNameFromPath(GEngine::GetTextureBank()->GetTexturePath(static_cast<U32>(environmentLightComponent->AmbientCubemapReference))));
			modalNameToOpen = SelectTextureAssetModalName;
		}
		else if (SSpriteComponent* spriteComponent = dynamic_cast<SSpriteComponent*>(result.ComponentViewed))
		{
			assetNames.push_back(UGeneralUtils::ExtractFileBaseNameFromPath(GEngine::GetTextureBank()->GetTexturePath(static_cast<U32>(spriteComponent->TextureIndex))));
			modalNameToOpen = SelectTextureAssetModalName;
		}

		IterateAssetRepresentations(result, assetNames, assetLabels, modalNameToOpen);
	}

	void CInspectorWindow::IterateAssetRepresentations(const SComponentViewResult& result, const std::vector<std::string>& assetNames, const std::vector<std::string>& assetLabels, const std::string& modalNameToOpen)
	{
		for (U8 index = 0; index < static_cast<U8>(assetNames.size()); index++)
		{
			std::string assetName = assetNames[index];

			if (assetName.empty())
				assetName = "M_Checkboard_128x128";

			SEditorAssetRepresentation* assetRep = Manager->GetAssetRepFromName(assetName).get();

			GUI::Separator();

			if (assetLabels.size() > index)
				GUI::Text(assetLabels[index].c_str());
			else if (assetRep->Name.empty())
				GUI::Text("N/A");
			else
				GUI::Text(assetRep->Name.c_str());

			// TODO.NR: Update ImGui and use the overload that takes a unique identifier. This currently breaks when using two buttons with the same texture.
			if (GUI::ImageButton(assetName.c_str(), (SGuiTextureID)(intptr_t)assetRep->TextureRef, {GUI::TexturePreviewSizeX, GUI::TexturePreviewSizeY}))
			{
				// TODO.NR: Make Algo lib function that finds index of object in array, make for-each loop
				AssetPickedIndex = index;
				GUI::OpenPopup(modalNameToOpen.c_str());
				GUI::SetNextWindowPos(GUI::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, SVector2<F32>(0.5f, 0.5f));
			}
		}

		if (modalNameToOpen == SelectMeshAssetModalName)
			OpenSelectMeshAssetModal(result);
		else if (modalNameToOpen == SelectMaterialAssetModalName)
			OpenSelectMaterialAssetModal(result);
		else if (modalNameToOpen == SelectTextureAssetModalName)
			OpenSelectTextureAssetModal(result);
	}

	void CInspectorWindow::OpenSelectMeshAssetModal(const SComponentViewResult& result)
	{
		if (!GUI::BeginPopupModal("Select Mesh Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		SStaticMeshComponent* staticMesh = static_cast<SStaticMeshComponent*>(result.ComponentViewed);
		if (staticMesh == nullptr)
			return;

		F32 thumbnailPadding = 4.0f;
		F32 cellWidth = GUI::TexturePreviewSizeX * 0.75f + thumbnailPadding;
		F32 panelWidth = 256.0f;
		I32 columnCount = static_cast<I32>(panelWidth / cellWidth);
		U32 id = 0;

		if (!GUI::BeginTable("NewMeshAssetTable", columnCount))
		{
			GUI::EndPopup();
			return;
		}

		Manager->SetIsModalOpen(true);

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

			GUI::TableNextColumn();
			GUI::PushID(id++);

			if (GUI::ImageButton(assetRep->Name.c_str(), (SGuiTextureID)(intptr_t)assetRep->TextureRef, {GUI::TexturePreviewSizeX * 0.75f, GUI::TexturePreviewSizeY * 0.75f}))
			{
				Manager->GetRenderManager()->TryLoadStaticMeshComponent(assetRep->Name, staticMesh);

				SMaterialComponent* materialComp = Scene->GetComponent<SMaterialComponent>(staticMesh);
				if (materialComp != nullptr)
				{
					U8 meshMaterialNumber = staticMesh->NumberOfMaterials;
					I8 materialNumberDifference = meshMaterialNumber - static_cast<U8>(materialComp->Materials.size());

					// NR: Add materials to correspond with mesh
					if (materialNumberDifference > 0)
					{
						for (U8 i = 0; i < materialNumberDifference; i++)
						{
							materialComp->Materials.emplace_back();
						}
					}
					// NR: Remove materials to correspond with mesh
					else if (materialNumberDifference < 0)
					{
						for (U8 i = 0; i < materialNumberDifference * -1.0f; i++)
						{
							materialComp->Materials.pop_back();
						}
					}
					// NR: Do nothing
					else
					{ 
					}
				}

				Manager->SetIsModalOpen(false);
				GUI::CloseCurrentPopup();
			}

			GUI::Text(assetRep->Name.c_str());
			GUI::PopID();
		}

		GUI::EndTable();

		if (GUI::Button("Cancel", SVector2<F32>(GUI::GetContentRegionAvail().x, 0))
		{
			Manager->SetIsModalOpen(false);
			GUI::CloseCurrentPopup(); 
		}

		GUI::EndPopup();
	}

	void CInspectorWindow::OpenSelectTextureAssetModal(const SComponentViewResult& result)
	{
		if (!GUI::BeginPopupModal("Select Texture Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			return;
		
		constexpr const char* searchPath = "Assets/Textures";

		if (SDecalComponent* decalComponent = dynamic_cast<SDecalComponent*>(result.ComponentViewed))
			HandleTextureAssetModal(searchPath, decalComponent->TextureReferences[result.ComponentSubIndex]);

		if (SEnvironmentLightComponent* environmentLightComponent = dynamic_cast<SEnvironmentLightComponent*>(result.ComponentViewed))
			HandleTextureAssetModal(searchPath, environmentLightComponent->AmbientCubemapReference);

		if (SSpriteComponent* spriteComponent = dynamic_cast<SSpriteComponent*>(result.ComponentViewed))
			HandleTextureAssetModal(searchPath, spriteComponent->TextureIndex);

		if (GUI::Button("Cancel", SVector2<F32>(GUI::GetContentRegionAvail().x, 0)))
		{
			Manager->SetIsModalOpen(false);
			GUI::CloseCurrentPopup();
		}

		GUI::EndPopup();
	}

	void CInspectorWindow::OpenSelectMaterialAssetModal(const SComponentViewResult& result)
	{
		if (!GUI::BeginPopupModal("Select Material Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		SMaterialComponent* materialComponent = dynamic_cast<SMaterialComponent*>(result.ComponentViewed);
		if (materialComponent == nullptr)
			return;

		F32 thumbnailPadding = 4.0f;
		F32 cellWidth = GUI::TexturePreviewSizeX * 0.75f + thumbnailPadding;
		F32 panelWidth = 256.0f;
		I32 columnCount = static_cast<I32>(panelWidth / cellWidth);
		U32 id = 0;

		Manager->SetIsModalOpen(true);

		if (GUI::BeginTable("NewMaterialAssetTable", columnCount))
		{
			for (auto& entry : std::filesystem::recursive_directory_iterator("Assets/Materials"))
			{
				if (entry.is_directory())
					continue;

				auto& assetRep = Manager->GetAssetRepFromDirEntry(entry);

				GUI::TableNextColumn();
				GUI::PushID(id++);

				if (GUI::ImageButton(assetRep->Name.c_str(), (SGuiTextureID)(intptr_t)assetRep->TextureRef, {GUI::TexturePreviewSizeX * 0.75f, GUI::TexturePreviewSizeY * 0.75f}))
				{
					Manager->GetRenderManager()->TryReplaceMaterialOnComponent(assetRep->DirectoryEntry.path().string(), AssetPickedIndex, materialComponent);
					AssetPickedIndex = 0;

					Manager->SetIsModalOpen(false);
					GUI::CloseCurrentPopup();
				}

				GUI::Text(assetRep->Name.c_str());
				GUI::PopID();
			}

			GUI::EndTable();
		}

		if (GUI::Button("Cancel", SVector2<F32>(GUI::GetContentRegionAvail().x, 0))) 
		{
			Manager->SetIsModalOpen(false);
			GUI::CloseCurrentPopup(); 
		}

		GUI::EndPopup();	
	}

	void CInspectorWindow::OpenAssetTool(const SComponentViewResult& result)
	{
		SSpriteAnimatorGraphComponent* component = static_cast<SSpriteAnimatorGraphComponent*>(result.ComponentViewed);
		if (component == nullptr)
			return;

		Manager->GetEditorWindow<CSpriteAnimatorGraphNodeWindow>()->Inspect(*component);
	}

	void CInspectorWindow::HandleTextureAssetModal(const std::string& pathToSearch, U16& textureReference)
	{
		F32 cellWidth = GUI::TexturePreviewSizeX * 0.75f + GUI::ThumbnailPadding;
		I32 columnCount = static_cast<I32>(GUI::PanelWidth / cellWidth);
		U32 id = 0;

		Manager->SetIsModalOpen(true);

		if (GUI::BeginTable("NewTextureAssetTable", columnCount))
		{
			for (auto& entry : std::filesystem::recursive_directory_iterator(pathToSearch.c_str()))
			{
				if (entry.is_directory())
					continue;

				auto& assetRep = Manager->GetAssetRepFromDirEntry(entry);

				GUI::TableNextColumn();
				GUI::PushID(id++);

				if (GUI::ImageButton(assetRep->Name.c_str(), (SGuiTextureID)(intptr_t)assetRep->TextureRef, {GUI::TexturePreviewSizeX * 0.75f, GUI::TexturePreviewSizeY * 0.75f}))
				{
					textureReference = static_cast<U16>(GEngine::GetTextureBank()->GetTextureIndex(entry.path().string()));
					
					Manager->SetIsModalOpen(false);
					GUI::CloseCurrentPopup();
				}

				GUI::Text(assetRep->Name.c_str());
				GUI::PopID();
			}

			GUI::EndTable();
		}
	}

	void CInspectorWindow::OpenAddComponentModal()
	{
		if (!GUI::BeginPopupModal("Add Component Modal", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		Manager->SetIsModalOpen(true);

		if (GUI::BeginTable("NewComponentTypeTable", 1))
		{
			for (const SComponentEditorContext* context : Scene->GetComponentEditorContexts())
			{
				GUI::TableNextColumn();

				if (context->AddComponent(SelectedEntity, Scene))
				{
					Manager->SetIsModalOpen(false);
					GUI::CloseCurrentPopup();
				}
			}

			GUI::EndTable();
		}

		if (GUI::Button("Cancel", ImVec2(GUI::GetContentRegionAvail().x, 0))) 
		{
			Manager->SetIsModalOpen(false);
			GUI::CloseCurrentPopup(); 
		}

		GUI::EndPopup();
	}
}
