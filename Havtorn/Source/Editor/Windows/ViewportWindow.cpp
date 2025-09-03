// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ViewportWindow.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderingPrimitives/FullscreenTexture.h"

#include <Scene/Scene.h>
#include <Scene/AssetRegistry.h>
#include <MathTypes/MathUtilities.h>
#include <PlatformManager.h>
#include <Input/Input.h>

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

		intptr_t playButtonID = (intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::PlayIcon).GetShaderResourceView();
		intptr_t pauseButtonID = (intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::PauseIcon).GetShaderResourceView();
		intptr_t stopButtonID = (intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::StopIcon).GetShaderResourceView();

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

		CAssetRegistry* assetRegistry = GEngine::GetWorld()->GetAssetRegistry();
		CRenderManager* renderManager = Manager->GetRenderManager();

		switch (assetRepresentation->AssetType)
		{
		case EAssetType::StaticMesh:
		{
			std::string staticMeshPath = assetRepresentation->Name + ".hva";
			renderManager->LoadStaticMeshComponent(staticMeshPath, scene->AddComponent<SStaticMeshComponent>(scene->PreviewEntity));
			scene->AddComponentEditorContext(scene->PreviewEntity, &SStaticMeshComponentEditorContext::Context);
			SStaticMeshComponent* staticMesh = scene->GetComponent<SStaticMeshComponent>(scene->PreviewEntity);
			staticMesh->AssetRegistryKey = assetRegistry->Register(staticMeshPath);

			std::vector<std::string> previewMaterials;
			previewMaterials.resize(staticMesh->NumberOfMaterials, CEditorManager::PreviewMaterial);
			renderManager->LoadMaterialComponent(previewMaterials, scene->AddComponent<SMaterialComponent>(scene->PreviewEntity));
			scene->AddComponentEditorContext(scene->PreviewEntity, &SMaterialComponentEditorContext::Context);
			scene->GetComponent<SMaterialComponent>(scene->PreviewEntity)->AssetRegistryKeys = assetRegistry->Register(previewMaterials);
		}
			break;

		case EAssetType::SkeletalMesh:
		{
			std::string meshPath = assetRepresentation->Name + ".hva";;
			renderManager->LoadSkeletalMeshComponent(meshPath, scene->AddComponent<SSkeletalMeshComponent>(scene->PreviewEntity));
			scene->AddComponentEditorContext(scene->PreviewEntity, &SSkeletalMeshComponentEditorContext::Context);
			SSkeletalMeshComponent* skeletalMesh = scene->GetComponent<SSkeletalMeshComponent>(scene->PreviewEntity);
			skeletalMesh->AssetRegistryKey = assetRegistry->Register(meshPath);
			
			// TODO.NW: Deal with different asset types, and figure out bind pose for skeletal meshes

			std::vector<std::string> previewMaterials;
			previewMaterials.resize(skeletalMesh->NumberOfMaterials, CEditorManager::PreviewMaterial);
			renderManager->LoadMaterialComponent(previewMaterials, scene->AddComponent<SMaterialComponent>(scene->PreviewEntity));
			scene->AddComponentEditorContext(scene->PreviewEntity, &SMaterialComponentEditorContext::Context);
			scene->GetComponent<SMaterialComponent>(scene->PreviewEntity)->AssetRegistryKeys = assetRegistry->Register(previewMaterials);
		}
			break;

		default :
			break;
		//std::string animationPath = "Assets/Tests/CH_Enemy_Walk.hva";
		////std::string animationPath = "Assets/Tests/MaleWave.hva";
		////std::string animationPath = "Assets/Tests/TestWalk.hva";
		////std::string animationPath = "Assets/Tests/DebugAnimAnim.hva";
		//renderManager->LoadSkeletalAnimationComponent(animationPath, scene->AddComponent<SSkeletalAnimationComponent>(scene->PreviewEntity));
		//scene->AddComponentEditorContext(scene->PreviewEntity, &SSkeletalAnimationComponentEditorContext::Context);
		}

	}
}