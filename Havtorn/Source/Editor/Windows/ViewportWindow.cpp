// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ViewportWindow.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderingPrimitives/RenderTexture.h"

#include <Scene/Scene.h>
#include <Assets/AssetRegistry.h>
#include <MathTypes/MathUtilities.h>
#include <PlatformManager.h>
#include <Input/Input.h>

namespace Havtorn
{
	CViewportWindow::CViewportWindow(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager)
		, RenderedSceneTextureReference(nullptr)
	{
		SnappingOptions.push_back({});
		SnappingOptions.emplace_back(SVector(0.01f), "0.01");
		SnappingOptions.emplace_back(SVector(0.05f), "0.05");
		SnappingOptions.emplace_back(SVector(0.1f), "0.1");
		SnappingOptions.emplace_back(SVector(0.5f), "0.5");
		SnappingOptions.emplace_back(SVector(1.0f), "1.0");
	}

	CViewportWindow::~CViewportWindow()
	{
	}

	void CViewportWindow::OnEnable()
	{
	}

	void CViewportWindow::OnInspectorGUI()
	{
		const SEditorLayout& layout = Manager->GetEditorLayout();

		SVector2<F32> viewportWorkPos = GUI::GetViewportWorkPos();
		GUI::SetNextWindowPos(SVector2<F32>(viewportWorkPos.X + layout.ViewportPosition.X, viewportWorkPos.Y + layout.ViewportPosition.Y));
		GUI::SetNextWindowSize(SVector2<F32>(layout.ViewportSize.X, layout.ViewportSize.Y));
		GUI::PushStyleVar(EStyleVar::WindowPadding, SVector2<F32>(0.0f));
		GUI::PushStyleVar(EStyleVar::ItemSpacing, SVector2<F32>(0.0f));

		const CEditorResourceManager* resourceManager = Manager->GetResourceManager();
		intptr_t playButtonID = (intptr_t)resourceManager->GetEditorTexture(EEditorTexture::PlayIcon).GetShaderResourceView();
		intptr_t pauseButtonID = (intptr_t)resourceManager->GetEditorTexture(EEditorTexture::PauseIcon).GetShaderResourceView();
		intptr_t stopButtonID = (intptr_t)resourceManager->GetEditorTexture(EEditorTexture::StopIcon).GetShaderResourceView();
		intptr_t settingsButtonID = (intptr_t)resourceManager->GetEditorTexture(EEditorTexture::EnvironmentLightIcon).GetShaderResourceView();

		if (GUI::Begin(Name(), nullptr, { EWindowFlag::NoMove, EWindowFlag::NoResize, EWindowFlag::NoCollapse, EWindowFlag::NoBringToFrontOnFocus }))
		{
			SVector2<F32> buttonSize = { 16.0f, 16.0f };
			std::vector<SAlignedButtonData> buttonData;
			buttonData.push_back({ [&]() { if (GEngine::GetWorld()->BeginPlay()) { IsPlayButtonEngaged = true; IsPauseButtonEngaged = false; } }, playButtonID, IsPlayButtonEngaged });
			buttonData.push_back({ [&]() { if (GEngine::GetWorld()->PausePlay()) { IsPlayButtonEngaged = false; IsPauseButtonEngaged = true; } }, pauseButtonID, IsPauseButtonEngaged });
			buttonData.push_back({ [&]() { if (GEngine::GetWorld()->StopPlay()) { IsPlayButtonEngaged = false; IsPauseButtonEngaged = false; } }, stopButtonID, false });
			GUI::AddViewportButtons(buttonData, buttonSize, layout.ViewportSize.X);
			
			// TODO.NW: Fix size of this button
			GUI::SameLine(layout.ViewportSize.X * 0.5f - 8.0f + 64.0f);
			std::string playDimensionLabel = GEngine::GetWorld()->GetWorldPlayDimensions() == EWorldPlayDimensions::World3D ? "3D" : "2D";
			if (GUI::Button(playDimensionLabel.c_str(), buttonSize + GUI::GetStyleVar(EStyleVar::FramePadding) * 2.0f))
			{
				GEngine::GetWorld()->ToggleWorldPlayDimensions();
			}

			GUI::SameLine(layout.ViewportSize.X * 0.5f - 8.0f + 96.0f);
			if (GUI::ImageButton("ViewportSettingsButton", settingsButtonID, buttonSize))
				GUI::OpenPopup("ViewportSettings");

			if (GUI::BeginPopup("ViewportSettings")) 
			{
				auto spaceLabels = magic_enum::enum_names<ETransformGizmoSpace>();
				auto currentLabel = magic_enum::enum_name(Manager->GetCurrentGizmoSpace());
				if (GUI::BeginCombo("Gizmo Space", currentLabel.data()))
				{
					for (auto label : spaceLabels)
					{
						bool isSelected = label == currentLabel;
						if (GUI::Selectable(label.data(), isSelected))
							Manager->SetGizmoSpace(magic_enum::enum_cast<ETransformGizmoSpace>(label).value());

						if (isSelected)
							GUI::SetItemDefaultFocus();
					}
					GUI::EndCombo();
				}

				// TODO.NW: Maybe we'd like to place this somewhere else?
				SSnappingOption currentSnapping = Manager->GetCurrentGizmoSnapping();
				if (GUI::BeginCombo("Snapping", currentSnapping.Label.c_str()))
				{
					for (auto& option : SnappingOptions)
					{
						bool isSelected = option == currentSnapping;
						if (GUI::Selectable(option.Label.c_str(), isSelected))
							Manager->SetGizmoSnapping(option);

						if (isSelected)
							GUI::SetItemDefaultFocus();
					}
					GUI::EndCombo();
				}

				GUI::EndPopup();
			}

			RenderedSceneTextureReference = &(Manager->GetRenderManager()->GetRenderedSceneTexture(0));
			
			if (RenderedSceneTextureReference)
			{
				SVector2<F32> vMin = GUI::GetWindowContentRegionMin();
				SVector2<F32> vMax = GUI::GetWindowContentRegionMax();

				F32 width = static_cast<F32>(vMax.X - vMin.X);
				F32 height = static_cast<F32>(vMax.Y - vMin.Y - ViewportMenuHeight - 4.0f);

				viewportWorkPos = GUI::GetViewportWorkPos();
				SVector2<F32> windowPos = SVector2<F32>(viewportWorkPos.X + layout.ViewportPosition.X, viewportWorkPos.Y + layout.ViewportPosition.Y);
				windowPos.Y += ViewportMenuHeight - 4.0f;
				RenderedScenePosition.X = windowPos.X;
				RenderedScenePosition.Y = windowPos.Y;
				RenderedSceneDimensions = { width, height };

				GUI::Image((intptr_t)RenderedSceneTextureReference->GetShaderResourceView(), SVector2<F32>(width, height));
			}
		
			GUI::SetGizmoDrawList();

			// TODO.NW: Unnestle these ifs
			CScene* firstActiveScene = GEngine::GetWorld()->GetActiveScenes().empty() ? nullptr : GEngine::GetWorld()->GetActiveScenes()[0].get();
			if (firstActiveScene != nullptr)
			{
				if (GUI::BeginDragDropTarget())
				{
					SGuiPayload payload = GUI::AcceptDragDropPayload("AssetDrag", { EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNopreviewTooltip });
					if (payload.Data != nullptr)
					{
						SEditorAssetRepresentation* payloadAssetRep = reinterpret_cast<SEditorAssetRepresentation*>(payload.Data);
						UpdatePreviewEntity(firstActiveScene, payloadAssetRep);
						
						if (firstActiveScene->PreviewEntity.IsValid())
							GUI::SetTooltip("Create Entity?");
						else
							GUI::SetTooltip("Asset type not supported yet!");

						if (payload.IsDelivery)
						{
							Manager->SetSelectedEntity(firstActiveScene->PreviewEntity);
							firstActiveScene->PreviewEntity = SEntity::Null;
						}
					}

					GUI::EndDragDropTarget();
				}
				else
				{
					if (firstActiveScene->PreviewEntity.IsValid())
					{
						// TODO.NW: Figure out mismatch that happens with other components when we remove the preview, suddenly entity component indices are one too big. Is it a race condition or something?
						firstActiveScene->RemoveEntity(firstActiveScene->PreviewEntity);
						firstActiveScene->PreviewEntity = SEntity::Null;
					}
				}
			}
		}

		GUI::PopStyleVar();
		GUI::PopStyleVar();
		GUI::End();
	}

	void CViewportWindow::OnDisable()
	{
	}
	
	const SVector2<F32> CViewportWindow::GetRenderedSceneDimensions() const
	{
		return RenderedSceneDimensions;
	}
	
	const SVector2<F32> CViewportWindow::GetRenderedScenePosition() const
	{
		return RenderedScenePosition;
	}

	void CViewportWindow::UpdatePreviewEntity(CScene* scene, const SEditorAssetRepresentation* assetRepresentation)
	{
		if (assetRepresentation->AssetType != EAssetType::StaticMesh && assetRepresentation->AssetType != EAssetType::SkeletalMesh)
			return;

		if (scene->PreviewEntity.IsValid())
		{
			// Handle transform
			
			CInput* input = CInput::GetInstance();
			F32 mouseX = STATIC_F32(input->GetMouseX());
			F32 mouseY = STATIC_F32(input->GetMouseY());

			SMatrix viewMatrix = scene->GetComponent<STransformComponent>(scene->MainCameraEntity)->Transform.GetMatrix();
			SMatrix projectionMatrix = scene->GetComponent<SCameraComponent>(scene->MainCameraEntity)->ProjectionMatrix;
			SRay worldRay = UMathUtilities::RaycastWorld({ mouseX, mouseY }, RenderedSceneDimensions, RenderedScenePosition, viewMatrix, projectionMatrix);

			// TODO.NW: This is too annoying, we should have an easy time of setting the transform of entities
			STransformComponent& previewTransform = *scene->GetComponent<STransformComponent>(scene->PreviewEntity);
			SMatrix transformCopy = previewTransform.Transform.GetMatrix();
			constexpr F32 dragDistanceFromEditorCamera = 3.0f;
			transformCopy.SetTranslation(worldRay.GetPointOnRay(dragDistanceFromEditorCamera));
			previewTransform.Transform.SetMatrix(transformCopy);
			return;
		}

		scene->PreviewEntity = scene->AddEntity("NewEntity");

		scene->AddComponent<STransformComponent>(scene->PreviewEntity)->Transform;
		scene->AddComponentEditorContext(scene->PreviewEntity, &STransformComponentEditorContext::Context);

		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();

		switch (assetRepresentation->AssetType)
		{
		case EAssetType::StaticMesh:
		{
			std::string staticMeshPath = assetRepresentation->DirectoryEntry.path().string();
			scene->AddComponent<SStaticMeshComponent>(scene->PreviewEntity, staticMeshPath);
			scene->AddComponentEditorContext(scene->PreviewEntity, &SStaticMeshComponentEditorContext::Context);
			SStaticMeshAsset* meshAsset = assetRegistry->RequestAssetData<SStaticMeshAsset>(SAssetReference(staticMeshPath), scene->PreviewEntity.GUID);

			std::vector<std::string> previewMaterials;
			previewMaterials.resize(meshAsset->NumberOfMaterials, CEditorManager::PreviewMaterial);
			scene->AddComponent<SMaterialComponent>(scene->PreviewEntity, previewMaterials);
			scene->AddComponentEditorContext(scene->PreviewEntity, &SMaterialComponentEditorContext::Context);
		}
			break;

		case EAssetType::SkeletalMesh:
		{
			std::string meshPath = assetRepresentation->DirectoryEntry.path().string();
			scene->AddComponent<SSkeletalMeshComponent>(scene->PreviewEntity, meshPath);
			scene->AddComponentEditorContext(scene->PreviewEntity, &SSkeletalMeshComponentEditorContext::Context);
			SSkeletalMeshAsset* meshAsset = assetRegistry->RequestAssetData<SSkeletalMeshAsset>(SAssetReference(meshPath), scene->PreviewEntity.GUID);
			
			// TODO.NW: Deal with different asset types, and figure out bind pose for skeletal meshes

			std::vector<std::string> previewMaterials;
			previewMaterials.resize(meshAsset->NumberOfMaterials, CEditorManager::PreviewMaterial);
			scene->AddComponent<SMaterialComponent>(scene->PreviewEntity, previewMaterials);
			scene->AddComponentEditorContext(scene->PreviewEntity, &SMaterialComponentEditorContext::Context);
		}
			break;

		default :
			break;
		//std::string animationPath = "Assets/Meshes/CH_Enemy_Walk.hva";
		////std::string animationPath = "Assets/Meshes/MaleWave.hva";
		////std::string animationPath = "Assets/Meshes/TestWalk.hva";
		////std::string animationPath = "Assets/Meshes/DebugAnimAnim.hva";
		//renderManager->LoadSkeletalAnimationComponent(animationPath, scene->AddComponent<SSkeletalAnimationComponent>(scene->PreviewEntity));
		//scene->AddComponentEditorContext(scene->PreviewEntity, &SSkeletalAnimationComponentEditorContext::Context);
		}

	}
}