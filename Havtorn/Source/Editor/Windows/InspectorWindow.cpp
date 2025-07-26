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
#include "EditorResourceManager.h"

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

		const SVector2<F32>& viewportWorkPos = GUI::GetViewportWorkPos();
		GUI::SetNextWindowPos(SVector2<F32>(viewportWorkPos.X + layout.InspectorPosition.X, viewportWorkPos.Y + layout.InspectorPosition.Y));
		GUI::SetNextWindowSize(SVector2<F32>(layout.InspectorSize.X, layout.InspectorSize.Y));

		if (!GUI::Begin(Name(), nullptr, { EWindowFlag::NoMove, EWindowFlag::NoResize, EWindowFlag::NoCollapse, EWindowFlag::NoBringToFrontOnFocus }))
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

		std::vector<SEntity> selectedEntities = Manager->GetSelectedEntities();

		if (selectedEntities.empty())
		{
			GUI::End();
			return;
		}

		// TODO.NW: Go through and make sure everything in the inspector gets unique ID, maybe based on entity GUID. 
		// Don't want same IDs over a frame when multiple entities are selected
		for (const SEntity& selectedEntity : selectedEntities)
		{
			if (GUI::BeginDragDropSource())
			{
				SGuiPayload payload = GUI::GetDragDropPayload();
				if (payload.Data == nullptr)
				{
					GUI::SetDragDropPayload("EntityDrag", &selectedEntity, sizeof(SEntity));
				}
				GUI::Text("entityName");

				GUI::EndDragDropSource();
			}

			SMetaDataComponent* metaDataComp = Scene->GetComponent<SMetaDataComponent>(selectedEntity);
			if (metaDataComp != nullptr)
			{
				GUI::InputText("##MetaDataCompName", &metaDataComp->Name);
				GUI::SameLine();
				GUI::TextDisabled("GUID %u", metaDataComp->Owner.GUID);
				if (GUI::IsItemHovered())
					GUI::SetTooltip("GUID %u", metaDataComp->Owner.GUID);
			}
			GUI::Separator();

			for (SComponentEditorContext* context : Scene->GetComponentEditorContexts(selectedEntity))
			{
				if (context->RemoveComponent(selectedEntity, Scene))
					continue;

				GUI::SameLine();
				SComponentViewResult result = context->View(selectedEntity, Scene);

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
			if (GUI::Button("Add Component", SVector2<F32>(GUI::GetContentRegionAvail().X, 0)))
				GUI::OpenPopup("Add Component Modal");

			OpenAddComponentModal(selectedEntity);
		}

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

		GUI::SetRect(viewportWindowPosition, viewportWindowDimensions);
		
		SCameraComponent* cameraComp = Scene->GetComponent<SCameraComponent>(Scene->MainCameraEntity);
		STransformComponent* cameraTransformComp = Scene->GetComponent<STransformComponent>(Scene->MainCameraEntity);
		SMatrix viewMatrix = cameraTransformComp->Transform.GetMatrix();
		SMatrix inverseView = viewMatrix.Inverse();

		ViewManipulation(viewMatrix, viewportWindowPosition, viewportWindowDimensions);

		GUI::PushID(0);
		SMatrix transformMatrix = viewedTransformComp->Transform.GetMatrix();

		// NW: We can choose GetSelectedEntity (which returns the first selected entity) to base the gizmo on if we want. 
		// I think it feels nicer to get the gizmo on the latest selected entity though, even though it will apply the 
		// delta matrix one frame later on all the other entities. This probably doesn't matter in editor.
		if (viewedTransformComp->Owner == Manager->GetSelectedEntities().back())
		{
			GUI::GizmoManipulate(inverseView.data, cameraComp->ProjectionMatrix.data, Manager->GetCurrentGizmo(), ETransformGizmoSpace::World, transformMatrix.data, DeltaMatrix.data, NULL);
		}
		else
		{
			transformMatrix *= DeltaMatrix;
		}
		
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
		GUI::ViewManipulate(outCameraView.data, camDistance, SVector2<F32>(viewManipulateRight - size, viewManipulateTop), SVector2<F32>(size, size), SColor::FromPackedU32(backgroundColor));
	}

	void CInspectorWindow::InspectAssetComponent(const SComponentViewResult& result)
	{
		std::vector<std::string> assetNames = {};
		std::vector<std::string> assetLabels = {};
		std::string modalNameToOpen = "";
		std::string defaultAssetDirectory = "";

		if (SStaticMeshComponent* staticMeshComponent = dynamic_cast<SStaticMeshComponent*>(result.ComponentViewed))
		{
			assetNames.push_back(UGeneralUtils::ExtractFileBaseNameFromPath(staticMeshComponent->Name.AsString()));
			modalNameToOpen = SelectMeshAssetModalName;
			defaultAssetDirectory = DefaultMeshAssetDirectory;
		}
		else if (SMaterialComponent* materialComponent = dynamic_cast<SMaterialComponent*>(result.ComponentViewed))
		{
			if(materialComponent->Materials.size() == 0)
				assetNames.push_back("");
			
			for (auto& material : materialComponent->Materials)
				assetNames.push_back(material.Name);

			modalNameToOpen = SelectMaterialAssetModalName;
			defaultAssetDirectory = DefaultMaterialAssetDirectory;
		}
		else if (SDecalComponent* decalComponent = dynamic_cast<SDecalComponent*>(result.ComponentViewed))
		{
			for (U16 textureRef : decalComponent->TextureReferences)
				assetNames.push_back(UGeneralUtils::ExtractFileBaseNameFromPath(GEngine::GetTextureBank()->GetTexturePath(static_cast<U32>(textureRef))));

			modalNameToOpen = SelectTextureAssetModalName;
			defaultAssetDirectory = DefaultTextureAssetDirectory;
			assetLabels.push_back("Albedo");
			assetLabels.push_back("Material");
			assetLabels.push_back("Normal");
			// TODO.NW: Figure out the nicest way to deal with labels vs asset names. Would be cool with a drop down similar to UE. See Notes
		}
		else if (SEnvironmentLightComponent* environmentLightComponent = dynamic_cast<SEnvironmentLightComponent*>(result.ComponentViewed))
		{
			assetNames.push_back(UGeneralUtils::ExtractFileBaseNameFromPath(GEngine::GetTextureBank()->GetTexturePath(static_cast<U32>(environmentLightComponent->AmbientCubemapReference))));
			modalNameToOpen = SelectTextureAssetModalName;
			defaultAssetDirectory = DefaultTextureAssetDirectory;
		}
		else if (SSpriteComponent* spriteComponent = dynamic_cast<SSpriteComponent*>(result.ComponentViewed))
		{
			assetNames.push_back(UGeneralUtils::ExtractFileBaseNameFromPath(GEngine::GetTextureBank()->GetTexturePath(static_cast<U32>(spriteComponent->TextureIndex))));
			modalNameToOpen = SelectTextureAssetModalName;
			defaultAssetDirectory = DefaultTextureAssetDirectory;
		}
		else if (SScriptComponent* scriptComponent = dynamic_cast<SScriptComponent*>(result.ComponentViewed))
		{
			assetNames.push_back(scriptComponent->Script ? UGeneralUtils::ExtractFileBaseNameFromPath(scriptComponent->Script->FileName) : "Script");
			modalNameToOpen = SelectScriptAssetModalName;
			defaultAssetDirectory = DefaultScriptAssetDirectory;
		}
		else if (SSkeletalMeshComponent* skeletonComponent = dynamic_cast<SSkeletalMeshComponent*>(result.ComponentViewed))
		{
			assetNames.push_back(UGeneralUtils::ExtractFileBaseNameFromPath(skeletonComponent->Name.AsString()));
			modalNameToOpen = SelectSkeletonAssetModalName;
			defaultAssetDirectory = DefaultMeshAssetDirectory;
		}
		else if (SSkeletalAnimationComponent* skeletalAnimationComponent = dynamic_cast<SSkeletalAnimationComponent*>(result.ComponentViewed))
		{
			assetNames.push_back(UGeneralUtils::ExtractFileBaseNameFromPath(skeletalAnimationComponent->AssetName));
			modalNameToOpen = SelectSkeletonAssetModalName;
			defaultAssetDirectory = DefaultMeshAssetDirectory;
		}

		IterateAssetRepresentations(result, assetNames, assetLabels, modalNameToOpen, defaultAssetDirectory);
	}

	void CInspectorWindow::IterateAssetRepresentations(const SComponentViewResult& result, const std::vector<std::string>& assetNames, const std::vector<std::string>& /*assetLabels*/, const std::string& modalNameToOpen, const std::string& defaultSearchDirectory)
	{
		F32 thumbnailPadding = 4.0f;
		F32 cellWidth = GUI::TexturePreviewSizeX * 0.75f + thumbnailPadding;
		F32 panelWidth = 256.0f;
		I32 columnCount = static_cast<I32>(panelWidth / cellWidth);

		if (assetNames.size() == 0)
		{		
			SAssetPickResult assetPickResult = GUI::AssetPicker("Select", modalNameToOpen.c_str(), 0, defaultSearchDirectory.c_str(), columnCount, Manager->GetAssetInspectFunction(), EAssetType::Material);
			if (assetPickResult.State == EAssetPickerState::Active)
				Manager->SetIsModalOpen(true);
			else if (assetPickResult.State == EAssetPickerState::Cancelled)
				Manager->SetIsModalOpen(false);
			else if (assetPickResult.State == EAssetPickerState::AssetPicked)
			{
				AssetPickedIndex = 0;
				auto pickedAsset = Manager->GetAssetRepFromDirEntry(assetPickResult.PickedEntry).get();

				// TODO.NW: Should figure out if we can send along an editor context function to determine what happens 
				// if an asset changes, instead of the inspector needing to know how to change the components
				if (modalNameToOpen == SelectMeshAssetModalName)
					HandleMeshAssetPicked(result, pickedAsset);
				else if (modalNameToOpen == SelectMaterialAssetModalName)
					HandleMaterialAssetPicked(result, pickedAsset);
				else if (modalNameToOpen == SelectTextureAssetModalName)
					HandleTextureAssetPicked(result, pickedAsset);
				else if (modalNameToOpen == SelectScriptAssetModalName)
					HandleScriptAssetPicked(result, pickedAsset);
				else if (modalNameToOpen == SelectSkeletonAssetModalName)
					HandleSkeletonAssetPicked(result, pickedAsset);
				else if (modalNameToOpen == SelectSkeletonAnimationAssetModalName)
					HandleSkeletalAnimationAssetPicked(result, pickedAsset);

				Manager->SetIsModalOpen(false);
			}
		}

		for (U8 index = 0; index < static_cast<U8>(assetNames.size()); index++)
		{
			std::string assetName = assetNames[index];

			if (assetName.empty())
				assetName = "M_Checkboard_128x128";

			SEditorAssetRepresentation* assetRep = Manager->GetAssetRepFromName(assetName).get();
			GUI::Separator();

			std::string id = assetName;
			id.append(std::to_string(index));
			GUI::PushID(id.c_str());

			SAssetPickResult assetPickResult = GUI::AssetPicker(assetName.c_str(), modalNameToOpen.c_str(), (intptr_t)assetRep->TextureRef.GetShaderResourceView(), defaultSearchDirectory.c_str(), columnCount, Manager->GetAssetInspectFunction(), assetRep->AssetType);

			if (assetPickResult.State == EAssetPickerState::Active)
				Manager->SetIsModalOpen(true);
			else if (assetPickResult.State == EAssetPickerState::Cancelled)
				Manager->SetIsModalOpen(false);
			else if (assetPickResult.State == EAssetPickerState::AssetPicked)
			{
				AssetPickedIndex = index;
				auto pickedAsset = Manager->GetAssetRepFromDirEntry(assetPickResult.PickedEntry).get();

				// TODO.NW: Should figure out if we can send along an editor context function to determine what happens 
				// if an asset changes, instead of the inspector needing to know how to change the components
				if (modalNameToOpen == SelectMeshAssetModalName)
					HandleMeshAssetPicked(result, pickedAsset);
				else if (modalNameToOpen == SelectMaterialAssetModalName)
					HandleMaterialAssetPicked(result, pickedAsset);
				else if (modalNameToOpen == SelectTextureAssetModalName)
					HandleTextureAssetPicked(result, pickedAsset);
				else if (modalNameToOpen == SelectScriptAssetModalName)
					HandleScriptAssetPicked(result, pickedAsset);
				else if (modalNameToOpen == SelectSkeletonAssetModalName)
					HandleSkeletonAssetPicked(result, pickedAsset);
				else if (modalNameToOpen == SelectSkeletonAnimationAssetModalName)
					HandleSkeletalAnimationAssetPicked(result, pickedAsset);

				Manager->SetIsModalOpen(false);
			}

			GUI::PopID();
		}
	}

	void CInspectorWindow::HandleMeshAssetPicked(const SComponentViewResult& result, const SEditorAssetRepresentation* assetRep)
	{
		SStaticMeshComponent* staticMesh = static_cast<SStaticMeshComponent*>(result.ComponentViewed);
		if (staticMesh == nullptr)
			return;

		// TODO.NW: Figure out why we did TryLoadStaticMeshComponent here before
		Manager->GetRenderManager()->LoadStaticMeshComponent(assetRep->Name + ".hva", staticMesh, Scene);
	}

	void CInspectorWindow::HandleSkeletonAssetPicked(const SComponentViewResult& result, const SEditorAssetRepresentation* assetRep)
	{
		SSkeletalMeshComponent* skeletalMesh = static_cast<SSkeletalMeshComponent*>(result.ComponentViewed);
		if (skeletalMesh == nullptr)
			return;

		// TODO.NW: Figure out why we did TryLoadStaticMeshComponent here before
		Manager->GetRenderManager()->LoadSkeletalMeshComponent(assetRep->Name + ".hva", skeletalMesh, Scene);
	}

	void CInspectorWindow::HandleSkeletalAnimationAssetPicked(const SComponentViewResult& result, const SEditorAssetRepresentation* assetRep)
	{
		SSkeletalAnimationComponent* skeletalMesh = static_cast<SSkeletalAnimationComponent*>(result.ComponentViewed);
		if (skeletalMesh == nullptr)
			return;

		// TODO.NW: Figure out why we did TryLoadStaticMeshComponent here before
		Manager->GetRenderManager()->LoadSkeletalAnimationComponent({ assetRep->Name + ".hva" }, skeletalMesh);
	}

	void CInspectorWindow::HandleMaterialAssetPicked(const SComponentViewResult& result, const SEditorAssetRepresentation* assetRep)
	{
		SMaterialComponent* materialComponent = dynamic_cast<SMaterialComponent*>(result.ComponentViewed);
		if (materialComponent == nullptr)
			return;

		// TODO.NW: This doesn't work for instanced entities yet. Need a solution for that
		Manager->GetRenderManager()->TryReplaceMaterialOnComponent(assetRep->DirectoryEntry.path().string(), AssetPickedIndex, materialComponent);
		AssetPickedIndex = 0;
	}

	void CInspectorWindow::HandleTextureAssetPicked(const SComponentViewResult& result, const SEditorAssetRepresentation* assetRep)
	{
		U16 textureReference = static_cast<U16>(GEngine::GetTextureBank()->GetTextureIndex(assetRep->DirectoryEntry.path().string()));

		if (SDecalComponent* decalComponent = dynamic_cast<SDecalComponent*>(result.ComponentViewed))
			decalComponent->TextureReferences[AssetPickedIndex] = textureReference;

		if (SEnvironmentLightComponent* environmentLightComponent = dynamic_cast<SEnvironmentLightComponent*>(result.ComponentViewed))
			environmentLightComponent->AmbientCubemapReference = textureReference;

		if (SSpriteComponent* spriteComponent = dynamic_cast<SSpriteComponent*>(result.ComponentViewed))
			spriteComponent->TextureIndex = textureReference;
	}

	void CInspectorWindow::HandleScriptAssetPicked(const SComponentViewResult& result, const SEditorAssetRepresentation* assetRep)
	{
		SScriptComponent* component = dynamic_cast<SScriptComponent*>(result.ComponentViewed);
		if (component == nullptr)
			return;

		component->Script = GEngine::GetWorld()->LoadScript(assetRep->DirectoryEntry.path().string());
		component->DataBindings.clear();
		component->DataBindings.resize(component->Script->DataBindings.size());

		for (U64 i = 0; i < component->DataBindings.size(); i++)
		{
			auto& scriptBinding = component->Script->DataBindings[i];
			auto& componentBinding = component->DataBindings[i];
			componentBinding = scriptBinding;
		}
	}

	void CInspectorWindow::OpenAssetTool(const SComponentViewResult& result)
	{
		SSpriteAnimatorGraphComponent* component = static_cast<SSpriteAnimatorGraphComponent*>(result.ComponentViewed);
		if (component == nullptr)
			return;

		Manager->GetEditorWindow<CSpriteAnimatorGraphNodeWindow>()->Inspect(*component);
	}

	void CInspectorWindow::OpenAddComponentModal(const SEntity& entity)
	{
		if (!GUI::BeginPopupModal("Add Component Modal", NULL, { EWindowFlag::AlwaysAutoResize }))
			return;

		Manager->SetIsModalOpen(true);

		if (GUI::BeginTable("NewComponentTypeTable", 1))
		{
			for (const SComponentEditorContext* context : Scene->GetComponentEditorContexts())
			{
				GUI::TableNextColumn();

				if (context->AddComponent(entity, Scene))
				{
					Manager->SetIsModalOpen(false);
					GUI::CloseCurrentPopup();
				}
			}

			GUI::EndTable();
		}

		if (GUI::Button("Cancel", SVector2<F32>(GUI::GetContentRegionAvail().X, 0))) 
		{
			Manager->SetIsModalOpen(false);
			GUI::CloseCurrentPopup(); 
		}

		GUI::EndPopup();
	}
}
