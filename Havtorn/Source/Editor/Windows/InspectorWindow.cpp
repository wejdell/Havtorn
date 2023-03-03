// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "InspectorWindow.h"
#include "Core/ImGuizmo/ImGuizmo.h"

#include "ECS/ECSInclude.h"
#include "Engine.h"
#include "EditorManager.h"
#include "Graphics/RenderManager.h"
#include "Graphics/TextureBank.h"
#include "Scene/Scene.h"

#include "Windows/ViewportWindow.h"

namespace ImGui
{
	using Havtorn::SEditorLayout;
	using Havtorn::SVector;
	using Havtorn::SVector4;
	using Havtorn::SVector2;
	using Havtorn::EComponentType;
	using Havtorn::U64;
	using Havtorn::F32;

	CInspectorWindow::CInspectorWindow(const char* name, Havtorn::CEditorManager* manager)
		: CWindow(name, manager)
	{
		InspectionFunctions[EComponentType::TransformComponent]			= std::bind(&CInspectorWindow::InspectTransformComponent, this);
		InspectionFunctions[EComponentType::StaticMeshComponent]		= std::bind(&CInspectorWindow::InspectStaticMeshComponent, this);
		InspectionFunctions[EComponentType::CameraComponent]			= std::bind(&CInspectorWindow::InspectCameraComponent, this);
		InspectionFunctions[EComponentType::CameraControllerComponent]	= std::bind(&CInspectorWindow::InspectCameraControllerComponent, this);
		InspectionFunctions[EComponentType::MaterialComponent]			= std::bind(&CInspectorWindow::InspectMaterialComponent, this);
		InspectionFunctions[EComponentType::EnvironmentLightComponent]	= std::bind(&CInspectorWindow::InspectEnvironmentLightComponent, this);
		InspectionFunctions[EComponentType::DirectionalLightComponent]	= std::bind(&CInspectorWindow::InspectDirectionalLightComponent, this);
		InspectionFunctions[EComponentType::PointLightComponent]		= std::bind(&CInspectorWindow::InspectPointLightComponent, this);
		InspectionFunctions[EComponentType::SpotLightComponent]			= std::bind(&CInspectorWindow::InspectSpotLightComponent, this);
		InspectionFunctions[EComponentType::VolumetricLightComponent]	= std::bind(&CInspectorWindow::InspectVolumetricLightComponent, this);
		InspectionFunctions[EComponentType::DecalComponent]				= std::bind(&CInspectorWindow::InspectDecalComponent, this);
	}

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

		if (!Scene)
		{
			Scene = Manager->GetCurrentScene();

			if (!Scene)
			{
				ImGui::End();
				return;
			}
		}

		const Havtorn::SEntity* selection = Manager->GetSelectedEntity();
		if (!selection)
		{
			ImGui::End();
			return;
		}

		const Havtorn::SMetaDataComponent& metaDataComp = Scene->GetMetaDataComponents()[Scene->GetSceneIndex(*selection)];
		const std::string selectionString = metaDataComp.IsInUse ? metaDataComp.Name.AsString() : "Entity";

		ImGui::TextColored((&ImGui::GetStyle())->Colors[ImGuiCol_HeaderActive], selectionString.c_str());
		ImGui::Separator();
		
		SelectedEntityIndex = Scene->GetSceneIndex(*selection);

		for (U64 i = 0; i < STATIC_U64(EComponentType::Count) - 2; i++)
		{
			TryInspectComponent(selection, static_cast<EComponentType>(i));
		}
			
		ImGui::End();
	}

	void CInspectorWindow::OnDisable()
	{
	}

	void CInspectorWindow::TryInspectComponent(const Havtorn::SEntity* selectedEntity, Havtorn::EComponentType componentType)
	{
		if (selectedEntity->HasComponent(componentType))
		{
			InspectionFunctions[componentType]();
			ImGui::Dummy({ DummySize.X, DummySize.Y });
		}
	}

	void CInspectorWindow::InspectTransformComponent()
	{
		if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
			return;

		Havtorn::SMatrix transformMatrix = Scene->GetTransformComponents()[SelectedEntityIndex].Transform.GetMatrix();

		F32 matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(transformMatrix.data, matrixTranslation, matrixRotation, matrixScale);
		ImGui::DragFloat3("Position", matrixTranslation, SlideSpeed);
		ImGui::DragFloat3("Rotation", matrixRotation, SlideSpeed);
		ImGui::DragFloat3("Scale", matrixScale, SlideSpeed);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, transformMatrix.data);
		Scene->GetTransformComponents()[SelectedEntityIndex].Transform.SetMatrix(transformMatrix);

		if (Manager->GetIsFreeCamActive())
			return;

		CViewportWindow* viewportWindow = Manager->GetViewportWindow();
		SVector2<F32> viewPortWindowDimensions = viewportWindow->GetRenderedSceneDimensions();
		SVector2<F32> viewPortWindowPosition = viewportWindow->GetRenderedScenePosition();

		ImGuizmo::SetDrawlist(viewportWindow->GetCurrentDrawList());
		ImGuizmo::SetRect(viewPortWindowPosition.X, viewPortWindowPosition.Y, viewPortWindowDimensions.X, viewPortWindowDimensions.Y);

		Havtorn::U64 mainCameraIndex = Scene->GetMainCameraIndex();
		auto& cameraComp = Scene->GetCameraComponents()[mainCameraIndex];
		auto& cameraTransformComp = Scene->GetTransformComponents()[mainCameraIndex];
		Havtorn::SMatrix inverseView = cameraTransformComp.Transform.GetMatrix().Inverse();

		ImGuizmo::Manipulate(inverseView.data, cameraComp.ProjectionMatrix.data, static_cast<ImGuizmo::OPERATION>(Manager->GetCurrentGizmo()), ImGuizmo::LOCAL, transformMatrix.data);
		Scene->GetTransformComponents()[SelectedEntityIndex].Transform.SetMatrix(transformMatrix);
	}

	void CInspectorWindow::InspectStaticMeshComponent()
	{
		if (!ImGui::CollapsingHeader("Static Mesh", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
			return;

		Havtorn::SStaticMeshComponent* staticMesh = &Scene->GetStaticMeshComponents()[SelectedEntityIndex];
		Havtorn::SEditorAssetRepresentation* assetRep = Manager->GetAssetRepFromName(staticMesh->Name.AsString()).get();

		if (ImGui::ImageButton(assetRep->TextureRef, { TexturePreviewSize.X, TexturePreviewSize.Y }))
		{
			ImGui::OpenPopup("Select Mesh Asset");
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		}
		ImGui::Text(assetRep->Name.c_str());
		ImGui::TextDisabled("Number Of Materials: %i", staticMesh->NumberOfMaterials);
		
		
		OpenSelectMeshAssetModal(SelectedEntityIndex);
	}

	void CInspectorWindow::InspectCameraComponent()
	{
		if (!ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
			return;
			
		auto& cameraComp = Scene->GetCameraComponents()[SelectedEntityIndex];
		ImGui::DragFloat("FOV", &cameraComp.FOV, SlideSpeed, 1.0f, 180.0f);
		ImGui::DragFloat("Aspect Ratio", &cameraComp.AspectRatio, SlideSpeed, 0.1f, 10.0f);
		ImGui::DragFloat("Near Clip Plane", &cameraComp.NearClip, SlideSpeed, 0.01f, cameraComp.FarClip - 1.0f);
		ImGui::DragFloat("Far Clip Plane", &cameraComp.FarClip, SlideSpeed, cameraComp.NearClip + 1.0f, 10000.0f);

		cameraComp.ProjectionMatrix = Havtorn::SMatrix::PerspectiveFovLH(Havtorn::UMath::DegToRad(cameraComp.FOV), cameraComp.AspectRatio, cameraComp.NearClip, cameraComp.FarClip);
	}

	void CInspectorWindow::InspectCameraControllerComponent()
	{
		if (!ImGui::CollapsingHeader("Camera Controller", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
			return;

		auto& cameraControllerComp = Scene->GetCameraControllerComponents()[SelectedEntityIndex];
		ImGui::DragFloat("Max Move Speed", &cameraControllerComp.MaxMoveSpeed, SlideSpeed, 0.1f, 10.0f);
		ImGui::DragFloat("Rotation Speed", &cameraControllerComp.RotationSpeed, SlideSpeed, 0.1f, 5.0f);
		ImGui::DragFloat("Acceleration Duration", &cameraControllerComp.AccelerationDuration, SlideSpeed * 0.1f, 0.1f, 5.0f);
	}

	void CInspectorWindow::InspectMaterialComponent()
	{
		Havtorn::SMaterialComponent* materialComp = &Scene->GetMaterialComponents()[SelectedEntityIndex];

		if (!ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
			return;

		for (Havtorn::U8 materialIndex = 0; materialIndex < materialComp->Materials.size(); materialIndex++)
		{
			Havtorn::SEditorAssetRepresentation* assetRep = Manager->GetAssetRepFromName(materialComp->Materials[materialIndex].Name).get();

			ImGui::Separator();

			if (assetRep->Name.size() > 0)
				ImGui::Text(assetRep->Name.c_str());
			else
				ImGui::Text("Empty Material");

			if (ImGui::ImageButton(assetRep->TextureRef, { TexturePreviewSize.X, TexturePreviewSize.Y }))
			{
				MaterialToChangeIndex = materialIndex;
				ImGui::OpenPopup("Select Material Asset");
				ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			}
		}
		
		OpenSelectMaterialAssetModal(materialComp, MaterialToChangeIndex);	
	}

	void CInspectorWindow::InspectEnvironmentLightComponent()
	{
		if (!ImGui::CollapsingHeader("Environment Light", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
			return;

		auto& environmentLightComp = Scene->GetEnvironmentLightComponents()[SelectedEntityIndex];

		Havtorn::U16 ref = environmentLightComp.AmbientCubemapReference;
			
		ImGui::Text("Ambient Static Cubemap");
		if (ImGui::ImageButton((void*)Havtorn::GEngine::GetTextureBank()->GetTexture(ref), { TexturePreviewSize.X, TexturePreviewSize.Y }))
		{
			ImGui::OpenPopup("Select Texture Asset");
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		}

		OpenSelectTextureAssetModal(environmentLightComp.AmbientCubemapReference);	
	}

	void CInspectorWindow::InspectDirectionalLightComponent()
	{
		if (!ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
			return;

		auto& directionalLightComp = Scene->GetDirectionalLightComponents()[SelectedEntityIndex];
			
		Havtorn::F32 colorData[3] = { directionalLightComp.Color.X, directionalLightComp.Color.Y, directionalLightComp.Color.Z };
		ImGui::ColorPicker3("Color", colorData);
		directionalLightComp.Color.X = colorData[0];
		directionalLightComp.Color.Y = colorData[1];
		directionalLightComp.Color.Z = colorData[2];

		const SVector4 direction = directionalLightComp.Direction;
		Havtorn::F32 dirData[3] = { direction.X, direction.Y, direction.Z };
		ImGui::DragFloat3("Direction", dirData, SlideSpeed);
		directionalLightComp.Direction = { dirData[0], dirData[1], dirData[2], 0.0f };

		ImGui::DragFloat("Intensity", &directionalLightComp.Color.W, SlideSpeed);	
	}

	void CInspectorWindow::InspectPointLightComponent()
	{
		if (!ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
			return;

		auto& pointLightComp = Scene->GetPointLightComponents()[SelectedEntityIndex];

		Havtorn::F32 colorData[3] = { pointLightComp.ColorAndIntensity.X, pointLightComp.ColorAndIntensity.Y, pointLightComp.ColorAndIntensity.Z };
		ImGui::ColorPicker3("Color", colorData);
		pointLightComp.ColorAndIntensity.X = colorData[0];
		pointLightComp.ColorAndIntensity.Y = colorData[1];
		pointLightComp.ColorAndIntensity.Z = colorData[2];

		ImGui::DragFloat("Intensity", &pointLightComp.ColorAndIntensity.W, SlideSpeed);
		ImGui::DragFloat("Range", &pointLightComp.Range, SlideSpeed, 0.1f, 100.0f);
	}

	void CInspectorWindow::InspectSpotLightComponent()
	{
		if (!ImGui::CollapsingHeader("Spot Light", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
			return;

		auto& spotLightComp = Scene->GetSpotLightComponents()[SelectedEntityIndex];

		Havtorn::F32 colorData[3] = { spotLightComp.ColorAndIntensity.X, spotLightComp.ColorAndIntensity.Y, spotLightComp.ColorAndIntensity.Z };
		ImGui::ColorPicker3("Color", colorData);
		spotLightComp.ColorAndIntensity.X = colorData[0];
		spotLightComp.ColorAndIntensity.Y = colorData[1];
		spotLightComp.ColorAndIntensity.Z = colorData[2];

		ImGui::DragFloat("Intensity", &spotLightComp.ColorAndIntensity.W, SlideSpeed);
		ImGui::DragFloat("Range", &spotLightComp.Range, SlideSpeed, 0.1f, 100.0f);
		ImGui::DragFloat("Outer Angle", &spotLightComp.OuterAngle, SlideSpeed, spotLightComp.InnerAngle, 180.0f);
		ImGui::DragFloat("InnerAngle", &spotLightComp.InnerAngle, SlideSpeed, 0.0f, spotLightComp.OuterAngle - 0.01f);	
	}

	void CInspectorWindow::InspectVolumetricLightComponent()
	{
		if (!ImGui::CollapsingHeader("Volumetric Light", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
			return;

		auto& volumetricLightComp = Scene->GetVolumetricLightComponents()[SelectedEntityIndex];

		ImGui::Checkbox("Is Active", &volumetricLightComp.IsActive);
		ImGui::DragFloat("Number Of Samples", &volumetricLightComp.NumberOfSamples, SlideSpeed, 4.0f);

		volumetricLightComp.NumberOfSamples = Havtorn::UMath::Max(volumetricLightComp.NumberOfSamples, 4.0f);
		ImGui::DragFloat("Light Power", &volumetricLightComp.LightPower, SlideSpeed * 10000.0f, 0.0f);
		ImGui::DragFloat("Scattering Probability", &volumetricLightComp.ScatteringProbability, SlideSpeed * 0.1f, 0.0f, 1.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
		ImGui::DragFloat("Henyey-Greenstein G", &volumetricLightComp.HenyeyGreensteinGValue);
	}

	void CInspectorWindow::InspectDecalComponent()
	{
		if (!ImGui::CollapsingHeader("Decal", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
			return;

		auto& decalComp = Scene->GetDecalComponents()[SelectedEntityIndex];

		ImGui::Checkbox("Render Albedo", &decalComp.ShouldRenderAlbedo);
		ImGui::Checkbox("Render Material", &decalComp.ShouldRenderMaterial);
		ImGui::Checkbox("Render Normal", &decalComp.ShouldRenderNormal);

		for (Havtorn::U16 materialIndex = 0; materialIndex < decalComp.TextureReferences.size(); materialIndex++)
		{
			if (materialIndex % 3 == 0)
				ImGui::Text("Albedo");

			if (materialIndex % 3 == 1)
				ImGui::Text("Material");

			if (materialIndex % 3 == 2)
				ImGui::Text("Normal");

			Havtorn::U16 ref = decalComp.TextureReferences[materialIndex];
			if (ImGui::ImageButton((void*)Havtorn::GEngine::GetTextureBank()->GetTexture(ref), { TexturePreviewSize.X, TexturePreviewSize.Y }))
			{
				MaterialRefToChangeIndex = materialIndex;
				ImGui::OpenPopup("Select Texture Asset");
				ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			}
		}

		MaterialRefToChangeIndex = Havtorn::UMath::Min(MaterialRefToChangeIndex, static_cast<Havtorn::U16>(decalComp.TextureReferences.size() - 1));
		OpenSelectTextureAssetModal(decalComp.TextureReferences[MaterialRefToChangeIndex]);		
	}

	void CInspectorWindow::OpenSelectMeshAssetModal(Havtorn::I64 staticMeshComponentIndex)
	{
		if (!ImGui::BeginPopupModal("Select Mesh Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		Havtorn::SStaticMeshComponent* staticMesh = &Scene->GetStaticMeshComponents()[staticMeshComponentIndex];

		F32 thumbnailPadding = 4.0f;
		F32 cellWidth = TexturePreviewSize.X * 0.75f + thumbnailPadding;
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

			if (ImGui::ImageButton(assetRep->TextureRef, { TexturePreviewSize.X * 0.75f, TexturePreviewSize.Y * 0.75f }))
			{
				Manager->GetRenderManager()->TryLoadStaticMeshComponent(assetRep->Name, staticMesh);

				if (Scene->GetMaterialComponents()[staticMeshComponentIndex].IsInUse)
				{
					auto& materialComp = Scene->GetMaterialComponents()[staticMeshComponentIndex];

					Havtorn::U8 meshMaterialNumber = staticMesh->NumberOfMaterials;
					Havtorn::I8 materialNumberDifference = meshMaterialNumber - static_cast<Havtorn::U8>(materialComp.Materials.size());

					// NR: Add materials to correspond with mesh
					if (materialNumberDifference > 0)
					{
						for (Havtorn::U8 i = 0; i < materialNumberDifference; i++)
						{
							materialComp.Materials.emplace_back();
						}
					}
					// NR: Remove materials to correspond with mesh
					else if (materialNumberDifference < 0)
					{
						for (Havtorn::U8 i = 0; i < materialNumberDifference * -1.0f; i++)
						{
							materialComp.Materials.pop_back();
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

	void CInspectorWindow::OpenSelectTextureAssetModal(Havtorn::U16& textureRefToChange)
	{
		if (!ImGui::BeginPopupModal("Select Texture Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		F32 thumbnailPadding = 4.0f;
		F32 cellWidth = TexturePreviewSize.X * 0.75f + thumbnailPadding;
		F32 panelWidth = 256.0f;
		Havtorn::I32 columnCount = static_cast<Havtorn::I32>(panelWidth / cellWidth);
		Havtorn::U32 id = 0;

		if (ImGui::BeginTable("NewTextureAssetTable", columnCount))
		{
			for (auto& entry : std::filesystem::recursive_directory_iterator("Assets/Textures"))
			{
				if (entry.is_directory())
					continue;

				auto& assetRep = Manager->GetAssetRepFromDirEntry(entry);

				ImGui::TableNextColumn();
				ImGui::PushID(id++);

				if (ImGui::ImageButton(assetRep->TextureRef, { TexturePreviewSize.X * 0.75f, TexturePreviewSize.Y * 0.75f }))
				{
					textureRefToChange = static_cast<Havtorn::U16>(Havtorn::GEngine::GetTextureBank()->GetTextureIndex(entry.path().string()));
					ImGui::CloseCurrentPopup();
				}

				ImGui::Text(assetRep->Name.c_str());
				ImGui::PopID();
			}

			ImGui::EndTable();
		}

		if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x, 0))) 
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	void CInspectorWindow::OpenSelectMaterialAssetModal(Havtorn::SMaterialComponent* materialComponentToChange, Havtorn::U8 materialIndex)
	{
		if (!ImGui::BeginPopupModal("Select Material Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		F32 thumbnailPadding = 4.0f;
		F32 cellWidth = TexturePreviewSize.X * 0.75f + thumbnailPadding;
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

				if (ImGui::ImageButton(assetRep->TextureRef, { TexturePreviewSize.X * 0.75f, TexturePreviewSize.Y * 0.75f }))
				{

					Manager->GetRenderManager()->TryReplaceMaterialOnComponent(assetRep->DirectoryEntry.path().string(), materialIndex, materialComponentToChange);

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
}
