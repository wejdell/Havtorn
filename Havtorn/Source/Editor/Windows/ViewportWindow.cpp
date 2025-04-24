// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ViewportWindow.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderingPrimitives/FullscreenTexture.h"

#include <Scene/Scene.h>
#include <Scene/AssetRegistry.h>

namespace Havtorn
{
	CViewportWindow::CViewportWindow(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager)
		, RenderedSceneTextureReference(nullptr)
	{
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

		intptr_t playButtonID = (intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::PlayIcon);
		intptr_t pauseButtonID = (intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::PauseIcon);
		intptr_t stopButtonID = (intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::StopIcon);

		if (GUI::Begin(Name(), nullptr, { EWindowFlag::NoMove, EWindowFlag::NoResize, EWindowFlag::NoCollapse, EWindowFlag::NoBringToFrontOnFocus }))
		{
			SVector2<F32> buttonSize = { 16.0f, 16.0f };
			const std::vector<SColor>& colors = GUI::GetStyleColors();
			SColor buttonInactiveColor = colors[STATIC_U64(EStyleColor::Button)];
			SColor buttonActiveColor = colors[STATIC_U64(EStyleColor::ButtonActive)];
			SColor buttonHoveredColor = colors[STATIC_U64(EStyleColor::ButtonHovered)];
			SVector2<F32> uv0 = { 0.0f, 0.0f };
			SVector2<F32> uv1 = { 1.0f, 1.0f };

			// TODO.NR: Make an abstraction for what's happening inside and to these button blocks
			// TODO.NR: Make util button function based on GUI::ImageButtonEx that can fill the whole rect (not only image background) with the color we choose

			GUI::SameLine(layout.ViewportSize.X * 0.5f - 8.0f - 32.0f);
			SColor playButtonColor = IsPlayButtonEngaged ? buttonActiveColor : IsPlayButtonHovered ? buttonHoveredColor : buttonInactiveColor;
			if (GUI::ImageButton("PlayButton", playButtonID, buttonSize, uv0, uv1, playButtonColor))
			{
				if (GEngine::GetWorld()->BeginPlay())
				{
					IsPlayButtonEngaged = true;
					IsPauseButtonEngaged = false;
				}
			}
			IsPlayButtonHovered = GUI::IsItemHovered();

			GUI::SameLine(layout.ViewportSize.X * 0.5f - 8.0f);
			SColor pauseButtonColor = IsPauseButtonEngaged ? buttonActiveColor : IsPauseButtonHovered ? buttonHoveredColor : buttonInactiveColor;
			if (GUI::ImageButton("PauseButton", pauseButtonID, buttonSize, uv0, uv1, pauseButtonColor))
			{
				if (GEngine::GetWorld()->PausePlay())
				{
					IsPlayButtonEngaged = false;
					IsPauseButtonEngaged = true;
				}
			}
			IsPauseButtonHovered = GUI::IsItemHovered();

			GUI::SameLine(layout.ViewportSize.X * 0.5f - 8.0f + 32.0f);
			if (GUI::ImageButton("StopButton", stopButtonID, buttonSize))
			{
				if (GEngine::GetWorld()->StopPlay())
				{
					IsPlayButtonEngaged = false;
					IsPauseButtonEngaged = false;					
				}
			}

			// TODO.NW: Fix size of this button
			GUI::SameLine(layout.ViewportSize.X * 0.5f - 8.0f + 64.0f);
			std::string playDimensionLabel = GEngine::GetWorld()->GetWorldPlayDimensions() == EWorldPlayDimensions::World3D ? "3D" : "2D";
			if (GUI::Button(playDimensionLabel.c_str(), buttonSize + GUI::GetStyleVar(EStyleVar::FramePadding) * 2.0f))
			{
				GEngine::GetWorld()->ToggleWorldPlayDimensions();
			}

			RenderedSceneTextureReference = &(Manager->GetRenderManager()->GetRenderedSceneTexture());
			
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
						GUI::SetTooltip("Create Entity?");

						// NW: Respond to target, check type
						SEditorAssetRepresentation* payloadAssetRep = reinterpret_cast<SEditorAssetRepresentation*>(payload.Data);
						UpdatePreviewEntity(firstActiveScene, payloadAssetRep);

						if (payload.IsDelivery)
						{
							Manager->SetSelectedEntity(firstActiveScene->PreviewEntity);
							firstActiveScene->PreviewEntity = SEntity::Null;
						}
					}
					else
					{
						firstActiveScene->RemoveEntity(firstActiveScene->PreviewEntity);
					}

					GUI::EndDragDropTarget();
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
		if (scene->PreviewEntity.IsValid())
		{
			// Handle transform
			//scene->GetComponent<STransformComponent>(scene->PreviewEntity)
			return;
		}

		scene->PreviewEntity = scene->AddEntity("NewEntity");

		scene->AddComponent<STransformComponent>(scene->PreviewEntity)->Transform;
		scene->AddComponentEditorContext(scene->PreviewEntity, &STransformComponentEditorContext::Context);

		CAssetRegistry* assetRegistry = GEngine::GetWorld()->GetAssetRegistry();
		CRenderManager* renderManager = Manager->GetRenderManager();

		// Static Mesh
		//std::string staticMeshPath = "Assets/Tests/CH_Enemy.hva";
		std::string staticMeshPath = assetRepresentation->Name + ".hva";
		renderManager->LoadStaticMeshComponent(staticMeshPath, scene->AddComponent<SStaticMeshComponent>(scene->PreviewEntity));
		scene->AddComponentEditorContext(scene->PreviewEntity, &SStaticMeshComponentEditorContext::Context);
		SStaticMeshComponent* staticMesh = scene->GetComponent<SStaticMeshComponent>(scene->PreviewEntity);
		staticMesh->AssetRegistryKey = assetRegistry->Register(staticMeshPath);

		//// Skeletal Mesh
		//std::string meshPath = "Assets/Tests/CH_Enemy_SK.hva";
		////std::string meshPath = "Assets/Tests/MaleDefault.hva";
		////std::string meshPath = "Assets/Tests/DebugAnimMesh.hva";
		//renderManager->LoadSkeletalMeshComponent(meshPath, scene->AddComponent<SSkeletalMeshComponent>(scene->PreviewEntity));
		//scene->AddComponentEditorContext(scene->PreviewEntity, &SSkeletalMeshComponentEditorContext::Context);
		//scene->GetComponent<SSkeletalMeshComponent>(scene->PreviewEntity)->AssetRegistryKey = assetRegistry->Register(meshPath);

		//std::string animationPath = "Assets/Tests/CH_Enemy_Walk.hva";
		////std::string animationPath = "Assets/Tests/MaleWave.hva";
		////std::string animationPath = "Assets/Tests/TestWalk.hva";
		////std::string animationPath = "Assets/Tests/DebugAnimAnim.hva";
		//renderManager->LoadSkeletalAnimationComponent(animationPath, scene->AddComponent<SSkeletalAnimationComponent>(scene->PreviewEntity));
		//scene->AddComponentEditorContext(scene->PreviewEntity, &SSkeletalAnimationComponentEditorContext::Context);

		std::vector<std::string> previewMaterials;
		previewMaterials.resize(staticMesh->NumberOfMaterials, "Assets/Materials/M_Checkboard_128x128.hva");
		renderManager->LoadMaterialComponent(previewMaterials, scene->AddComponent<SMaterialComponent>(scene->PreviewEntity));
		scene->AddComponentEditorContext(scene->PreviewEntity, &SMaterialComponentEditorContext::Context);
		scene->GetComponent<SMaterialComponent>(scene->PreviewEntity)->AssetRegistryKeys = assetRegistry->Register(previewMaterials);
	}
}