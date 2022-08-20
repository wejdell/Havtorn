// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "InspectorWindow.h"
#include <imgui.h>

#include "ECS/ECSInclude.h"
#include "Engine.h"
#include "Editor/EditorManager.h"
#include "Graphics/RenderManager.h"
#include "Graphics/TextureBank.h"
#include "Scene/Scene.h"

namespace ImGui
{
	using Havtorn::SEditorLayout;
	using Havtorn::SVector;
	using Havtorn::SVector4;
	using Havtorn::SVector2;
	using Havtorn::EComponentType;
	using Havtorn::F32;

	CInspectorWindow::CInspectorWindow(const char* name, Havtorn::CScene* scene, Havtorn::CEditorManager* manager)
		: CWindow(name, manager)
		, Scene(scene)
	{
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

		if (ImGui::Begin(Name(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (const auto selection = Manager->GetSelectedEntity())
			{
				ImGui::TextColored((&ImGui::GetStyle())->Colors[ImGuiCol_HeaderActive], selection->Name.c_str());
				ImGui::Separator();

				if (selection->HasComponent(EComponentType::TransformComponent))
				{
					InspectTransformComponent(selection->GetComponentIndex(EComponentType::TransformComponent));
					ImGui::Dummy({ DummySize.X, DummySize.Y });
				}

				if (selection->HasComponent(EComponentType::StaticMeshComponent))
				{
					InspectStaticMeshComponent(selection->GetComponentIndex(EComponentType::StaticMeshComponent));
					ImGui::Dummy({ DummySize.X, DummySize.Y });
				}

				if (selection->HasComponent(EComponentType::CameraComponent))
				{
					InspectCameraComponent(selection->GetComponentIndex(EComponentType::CameraComponent));
					ImGui::Dummy({ DummySize.X, DummySize.Y });
				}

				if (selection->HasComponent(EComponentType::MaterialComponent))
				{
					InspectMaterialComponent(selection->GetComponentIndex(EComponentType::MaterialComponent));
					ImGui::Dummy({ DummySize.X, DummySize.Y });
				}

				if (selection->HasComponent(EComponentType::EnvironmentLightComponent))
				{
					InspectEnvironmentLightComponent(selection->GetComponentIndex(EComponentType::EnvironmentLightComponent));
					ImGui::Dummy({ DummySize.X, DummySize.Y });
				}

				if (selection->HasComponent(EComponentType::DirectionalLightComponent))
				{
					InspectDirectionalLightComponent(selection->GetComponentIndex(EComponentType::DirectionalLightComponent));
					ImGui::Dummy({ DummySize.X, DummySize.Y });
				}

				if (selection->HasComponent(EComponentType::PointLightComponent))
				{
					InspectPointLightComponent(selection->GetComponentIndex(EComponentType::PointLightComponent));
					ImGui::Dummy({ DummySize.X, DummySize.Y });
				}

				if (selection->HasComponent(EComponentType::SpotLightComponent))
				{
					InspectSpotLightComponent(selection->GetComponentIndex(EComponentType::SpotLightComponent));
					ImGui::Dummy({ DummySize.X, DummySize.Y });
				}

				if (selection->HasComponent(EComponentType::VolumetricLightComponent))
				{
					InspectVolumetricLightComponent(selection->GetComponentIndex(EComponentType::VolumetricLightComponent));
					ImGui::Dummy({ DummySize.X, DummySize.Y });
				}

				if (selection->HasComponent(EComponentType::DecalComponent))
				{
					InspectDecalComponent(selection->GetComponentIndex(EComponentType::DecalComponent));
					ImGui::Dummy({ DummySize.X, DummySize.Y });
				}
			}
		}
		ImGui::End();
	}

	void CInspectorWindow::OnDisable()
	{
	}

	void CInspectorWindow::InspectTransformComponent(Havtorn::I64 transformComponentIndex)
	{
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& transformMatrix = Scene->GetTransformComponents()[transformComponentIndex]->Transform.GetMatrix();
			const SVector position = transformMatrix.Translation();
			Havtorn::F32 data[3] = { position.X, position.Y, position.Z };
			ImGui::DragFloat3("Position", data, SlideSpeed);
			transformMatrix.Translation({ data[0], data[1], data[2] });
		}
	}

	void CInspectorWindow::InspectStaticMeshComponent(Havtorn::I64 staticMeshComponentIndex)
	{
		Havtorn::Ref<Havtorn::SStaticMeshComponent> staticMesh = nullptr;

		if (ImGui::CollapsingHeader("Static Mesh", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			staticMesh = Scene->GetStaticMeshComponents()[staticMeshComponentIndex];
			Havtorn::SEditorAssetRepresentation* assetRep = Manager->GetAssetRepFromName(staticMesh->Name).get();

			if (ImGui::ImageButton(assetRep->TextureRef, { TexturePreviewSize.X, TexturePreviewSize.Y }))
			{
				ImGui::OpenPopup("Select Mesh Asset");
				ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			}
			ImGui::Text(assetRep->Name.c_str());
			ImGui::TextDisabled("Number Of Materials: %i", staticMesh->NumberOfMaterials);
		}
		
		OpenSelectMeshAssetModal(staticMesh.get());
	}

	void CInspectorWindow::InspectCameraComponent(Havtorn::I64 cameraComponentIndex)
	{
		if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& cameraComp = Scene->GetCameraComponents()[cameraComponentIndex];
			ImGui::DragFloat("FOV", &cameraComp->FOV, SlideSpeed, 1.0f, 180.0f);
			ImGui::DragFloat("Aspect Ratio", &cameraComp->AspectRatio, SlideSpeed, 0.1f, 10.0f);
			ImGui::DragFloat("Near Clip Plane", &cameraComp->NearClip, SlideSpeed, 0.01f, cameraComp->FarClip - 1.0f);
			ImGui::DragFloat("Far Clip Plane", &cameraComp->FarClip, SlideSpeed, cameraComp->NearClip + 1.0f, 10000.0f);

			cameraComp->ProjectionMatrix = Havtorn::SMatrix::PerspectiveFovLH(Havtorn::UMath::DegToRad(cameraComp->FOV), cameraComp->AspectRatio, cameraComp->NearClip, cameraComp->FarClip);
		}
	}

	void CInspectorWindow::InspectMaterialComponent(Havtorn::I64 materialComponentIndex)
	{
		auto& materialComp = Scene->GetMaterialComponents()[materialComponentIndex];
		auto renderManager = Manager->GetRenderManager();

		if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			Havtorn::EMaterialConfiguration materialConfig = renderManager->GetMaterialConfiguration();

			for (Havtorn::U16 materialIndex = 0; materialIndex < materialComp->MaterialReferences.size(); materialIndex++)
			{
				switch (materialConfig)
				{
				case Havtorn::EMaterialConfiguration::AlbedoMaterialNormal_Packed:
				{
					if (materialIndex % 3 == 0)
					{
						ImGui::Text("Material %i", materialIndex / 3);
						ImGui::Separator();
						ImGui::Text("Albedo");
					}

					if (materialIndex % 3 == 1)
						ImGui::Text("Material");

					if (materialIndex % 3 == 2)
						ImGui::Text("Normal");
				}
					break;
				default:
					break;
				}

				Havtorn::U16 ref = materialComp->MaterialReferences[materialIndex];
				if (ImGui::ImageButton((void*)Havtorn::GEngine::GetTextureBank()->GetTexture(ref), { TexturePreviewSize.X, TexturePreviewSize.Y }))
				{
					MaterialRefToChangeIndex = materialIndex;
					ImGui::OpenPopup("Select Texture Asset");
					ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
				}
			}
		
			MaterialRefToChangeIndex = Havtorn::UMath::Min(static_cast<Havtorn::U16>(materialComp->MaterialReferences.size()), MaterialRefToChangeIndex);
			OpenSelectTextureAssetModal(materialComp->MaterialReferences[MaterialRefToChangeIndex]);
		}
	}

	void CInspectorWindow::InspectEnvironmentLightComponent(Havtorn::I64 environmentLightComponentIndex)
	{
		if (ImGui::CollapsingHeader("Environment Light", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& environmentLightComp = Scene->GetEnvironmentLightComponents()[environmentLightComponentIndex];

			Havtorn::U16 ref = environmentLightComp->AmbientCubemapReference;
			
			ImGui::Text("Ambient Static Cubemap");
			if (ImGui::ImageButton((void*)Havtorn::GEngine::GetTextureBank()->GetTexture(ref), { TexturePreviewSize.X, TexturePreviewSize.Y }))
			{
				ImGui::OpenPopup("Select Texture Asset");
				ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			}

			OpenSelectTextureAssetModal(environmentLightComp->AmbientCubemapReference);
		}
	}

	void CInspectorWindow::InspectDirectionalLightComponent(Havtorn::I64 directionalLightComponentIndex)
	{
		if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& directionalLightComp = Scene->GetDirectionalLightComponents()[directionalLightComponentIndex];
			
			Havtorn::F32 colorData[3] = { directionalLightComp->Color.X, directionalLightComp->Color.Y, directionalLightComp->Color.Z };
			ImGui::ColorPicker3("Color", colorData);
			directionalLightComp->Color.X = colorData[0];
			directionalLightComp->Color.Y = colorData[1];
			directionalLightComp->Color.Z = colorData[2];

			const SVector4 direction = directionalLightComp->Direction;
			Havtorn::F32 dirData[3] = { direction.X, direction.Y, direction.Z };
			ImGui::DragFloat3("Direction", dirData, SlideSpeed);
			directionalLightComp->Direction = { dirData[0], dirData[1], dirData[2], 0.0f };

			ImGui::DragFloat("Intensity", &directionalLightComp->Color.W, SlideSpeed);
		}
	}

	void CInspectorWindow::InspectPointLightComponent(Havtorn::I64 pointLightComponentIndex)
	{
		if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& pointLightComp = Scene->GetPointLightComponents()[pointLightComponentIndex];

			Havtorn::F32 colorData[3] = { pointLightComp->ColorAndIntensity.X, pointLightComp->ColorAndIntensity.Y, pointLightComp->ColorAndIntensity.Z };
			ImGui::ColorPicker3("Color", colorData);
			pointLightComp->ColorAndIntensity.X = colorData[0];
			pointLightComp->ColorAndIntensity.Y = colorData[1];
			pointLightComp->ColorAndIntensity.Z = colorData[2];

			ImGui::DragFloat("Intensity", &pointLightComp->ColorAndIntensity.W, SlideSpeed);
			ImGui::DragFloat("Range", &pointLightComp->Range, SlideSpeed, 0.1f, 100.0f);
		}
	}

	void CInspectorWindow::InspectSpotLightComponent(Havtorn::I64 spotLightComponentIndex)
	{
		if (ImGui::CollapsingHeader("Spot Light", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& spotLightComp = Scene->GetSpotLightComponents()[spotLightComponentIndex];

			Havtorn::F32 colorData[3] = { spotLightComp->ColorAndIntensity.X, spotLightComp->ColorAndIntensity.Y, spotLightComp->ColorAndIntensity.Z };
			ImGui::ColorPicker3("Color", colorData);
			spotLightComp->ColorAndIntensity.X = colorData[0];
			spotLightComp->ColorAndIntensity.Y = colorData[1];
			spotLightComp->ColorAndIntensity.Z = colorData[2];

			ImGui::DragFloat("Intensity", &spotLightComp->ColorAndIntensity.W, SlideSpeed);
			ImGui::DragFloat("Range", &spotLightComp->Range, SlideSpeed, 0.1f, 100.0f);
			ImGui::DragFloat("Outer Angle", &spotLightComp->OuterAngle, SlideSpeed, spotLightComp->InnerAngle, 180.0f);
			ImGui::DragFloat("InnerAngle", &spotLightComp->InnerAngle, SlideSpeed, 0.0f, spotLightComp->OuterAngle - 0.01f);
		}
	}

	void CInspectorWindow::InspectVolumetricLightComponent(Havtorn::I64 volumetricLightComponentIndex)
	{
		if (ImGui::CollapsingHeader("Volumetric Light", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& volumetricLightComp = Scene->GetVolumetricLightComponents()[volumetricLightComponentIndex];

			ImGui::Checkbox("Is Active", &volumetricLightComp->IsActive);
			ImGui::DragFloat("Number Of Samples", &volumetricLightComp->NumberOfSamples, SlideSpeed, 4.0f);
			volumetricLightComp->NumberOfSamples = max(volumetricLightComp->NumberOfSamples, 4.0f);
			ImGui::DragFloat("Light Power", &volumetricLightComp->LightPower, SlideSpeed * 10000.0f, 0.0f);
			ImGui::DragFloat("Scattering Probability", &volumetricLightComp->ScatteringProbability, SlideSpeed * 0.1f, 0.0f, 1.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
			ImGui::DragFloat("Henyey-Greenstein G", &volumetricLightComp->HenyeyGreensteinGValue);
		}
	}

	void CInspectorWindow::InspectDecalComponent(Havtorn::I64 decalComponentIndex)
	{
		if (ImGui::CollapsingHeader("Decal", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& decalComp = Scene->GetDecalComponents()[decalComponentIndex];

			ImGui::Checkbox("Render Albedo", &decalComp->ShouldRenderAlbedo);
			ImGui::Checkbox("Render Material", &decalComp->ShouldRenderMaterial);
			ImGui::Checkbox("Render Normal", &decalComp->ShouldRenderNormal);

			auto renderManager = Manager->GetRenderManager();

			Havtorn::EMaterialConfiguration materialConfig = renderManager->GetMaterialConfiguration();

			for (Havtorn::U16 materialIndex = 0; materialIndex < decalComp->TextureReferences.size(); materialIndex++)
			{
				switch (materialConfig)
				{
				case Havtorn::EMaterialConfiguration::AlbedoMaterialNormal_Packed:
				{
					if (materialIndex % 3 == 0)
						ImGui::Text("Albedo");

					if (materialIndex % 3 == 1)
						ImGui::Text("Material");

					if (materialIndex % 3 == 2)
						ImGui::Text("Normal");
				}
				break;
				default:
					break;
				}

				Havtorn::U16 ref = decalComp->TextureReferences[materialIndex];
				if (ImGui::ImageButton((void*)Havtorn::GEngine::GetTextureBank()->GetTexture(ref), { TexturePreviewSize.X, TexturePreviewSize.Y }))
				{
					MaterialRefToChangeIndex = materialIndex;
					ImGui::OpenPopup("Select Texture Asset");
					ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
				}
			}

			OpenSelectTextureAssetModal(decalComp->TextureReferences[MaterialRefToChangeIndex]);
		}
	}

	void CInspectorWindow::OpenSelectMeshAssetModal(Havtorn::SStaticMeshComponent* meshAssetToChange)
	{
		if (ImGui::BeginPopupModal("Select Mesh Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			F32 thumbnailPadding = 4.0f;
			F32 cellWidth = TexturePreviewSize.X * 0.75f + thumbnailPadding;
			F32 panelWidth = 256.0f;
			Havtorn::I32 columnCount = static_cast<Havtorn::I32>(panelWidth / cellWidth);
			Havtorn::U32 id = 0;

			if (ImGui::BeginTable("NewMeshAssetTable", columnCount))
			{
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
						Manager->GetRenderManager()->TryLoadStaticMeshComponent(assetRep->Name, meshAssetToChange);

						if (meshAssetToChange->Entity->HasComponent(EComponentType::MaterialComponent))
						{
							auto& materialComp = Scene->GetMaterialComponents()[meshAssetToChange->Entity->GetComponentIndex(EComponentType::MaterialComponent)];
							Havtorn::EMaterialConfiguration materialConfig = Manager->GetRenderManager()->GetMaterialConfiguration();
							Havtorn::U8 texturesPerMaterial = 0;
							switch (materialConfig)
							{
							case Havtorn::EMaterialConfiguration::AlbedoMaterialNormal_Packed:
								texturesPerMaterial = 3;
								break;
							default:
								break;
							}

							Havtorn::U8 materialReferencesByMesh = meshAssetToChange->NumberOfMaterials * texturesPerMaterial;
							Havtorn::I8 referenceDifference = materialReferencesByMesh - static_cast<Havtorn::U8>(materialComp->MaterialReferences.size());

							// NR: Add texture references to correspond with mesh
							if (referenceDifference > 0)
							{
								for (Havtorn::U8 i = 0; i < referenceDifference; i++)
								{
									materialComp->MaterialReferences.push_back(1);
								}
							}
							// NR: Remove texture references to correspond with mesh
							else if (referenceDifference < 0)
							{
								for (Havtorn::U8 i = 0; i < referenceDifference * -1.0f; i++)
								{
									materialComp->MaterialReferences.pop_back();
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
			}

			if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x, 0))) { ImGui::CloseCurrentPopup(); }

			ImGui::EndPopup();
		}
	}

	void CInspectorWindow::OpenSelectTextureAssetModal(Havtorn::U16& textureRefToChange)
	{
		if (ImGui::BeginPopupModal("Select Texture Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
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

			if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x, 0))) { ImGui::CloseCurrentPopup(); }

			ImGui::EndPopup();
		}
	}
}
