// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"

#include <FileSystem.h>
#include <Graphics/RenderManager.h>
#include <Graphics/GeometryPrimitives.h>
#include <Input/InputMapper.h>
#include <Input/InputTypes.h>
#include <Timer.h>
#include <Assets/AssetRegistry.h>
#include <ECS/Systems/CameraSystem.h>

#include "MaterialTool.h"

using Havtorn::I32;
using Havtorn::F32;
using Havtorn::U64;

namespace Havtorn
{
	using namespace HexRune;

	CMaterialTool::CMaterialTool(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager)
	{
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Zoom).AddMember(this, &CMaterialTool::OnZoomInput);
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::MouseDeltaHorizontal).AddMember(this, &CMaterialTool::HandleAxisInput);
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::MouseDeltaVertical).AddMember(this, &CMaterialTool::HandleAxisInput);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::ToggleFreeCam).AddMember(this, &CMaterialTool::ToggleFreeCam);
	}

	void CMaterialTool::OnEnable()
	{
	}

	void CMaterialTool::OnInspectorGUI()
	{
		// TODO.NW: Make ON_SCOPE_EXIT equivalent?
		if (!GUI::Begin(Name(), &IsEnabled))
		{
			GUI::End();
			return;
		}

		if (CurrentMaterial == nullptr)
		{
			GUI::End();
			return;
		}

		const bool isWindowHovered = GUI::IsWindowHovered();
		if (!IsHoveringWindow && isWindowHovered)
			GEngine::GetWorld()->BlockSystem<CCameraSystem>(this);
		else if (IsHoveringWindow && !isWindowHovered)
			GEngine::GetWorld()->UnblockSystem<CCameraSystem>(this);

		IsHoveringWindow = isWindowHovered;

		{ // Menu Bar
			GUI::BeginChild("MaterialOptions", SVector2<F32>(0.0f, 168.0f));
			GUI::Text(CurrentMaterial->Name.c_str());
			GUI::SameLine();

			GUI::SameLine();
			if (GUI::Button("Save"))
			{
				SMaterialAssetFileHeader asset;
				asset.Name = MaterialData.Name;
				asset.Material.RecreateZ = MaterialData.RecreateNormalZ;

				// TODO.NW: Need better way to transfer between runtime assets and disk, both ways
				auto fillProperty = [&](const SRuntimeGraphicsMaterialProperty& assetProperty, const I64 offlinePropertyIndex)
					{
						auto& offlineProperty = asset.Material.Properties[offlinePropertyIndex];
						offlineProperty.ConstantValue = assetProperty.ConstantValue;
						offlineProperty.TextureChannelIndex = STATIC_I16(assetProperty.TextureChannelIndex);
						if (assetProperty.TextureUID > 0)
							offlineProperty.TexturePath = GEngine::GetAssetRegistry()->GetAssetDatabaseEntry(STATIC_U32(assetProperty.TextureUID));
					};

				fillProperty(MaterialData.AlbedoR, 0);
				fillProperty(MaterialData.AlbedoG, 1);
				fillProperty(MaterialData.AlbedoB, 2);
				fillProperty(MaterialData.AlbedoA, 3);
				fillProperty(MaterialData.NormalX, 4);
				fillProperty(MaterialData.NormalY, 5);
				
				if (!asset.Material.RecreateZ)
					fillProperty(MaterialData.NormalZ, 6);
			
				fillProperty(MaterialData.AmbientOcclusion, 7);
				fillProperty(MaterialData.Metalness, 8);
				fillProperty(MaterialData.Roughness, 9);
				fillProperty(MaterialData.Emissive, 10);

				const auto data = new char[asset.GetSize()];

				asset.Serialize(data);
				const std::string currentMaterialPath = CurrentMaterial->DirectoryEntry.path().string();
				CurrentMaterial = nullptr;
				UFileSystem::Serialize(currentMaterialPath, &data[0], asset.GetSize());

				std::filesystem::directory_entry newDir;
				newDir.assign(std::filesystem::path(currentMaterialPath));
				Manager->RemoveAssetRep(newDir);
				Manager->CreateAssetRep(newDir);
				CurrentMaterial = Manager->GetAssetRepFromDirEntry(newDir).get();
			}

			GUI::Separator();

			{ // Preview Settings
				const SVector2<F32> settingsPropertySize = SVector2<F32>(256.0f, 136.0f);

				GUI::BeginChild("Cubemap", settingsPropertySize);
				auto skyboxAssetRep = Manager->GetAssetRepFromName(UGeneralUtils::ExtractFileBaseNameFromPath(PreviewSkylightAssetRef.FilePath)).get();

				intptr_t assetPickerThumbnail = skyboxAssetRep != nullptr ? (intptr_t)skyboxAssetRep->TextureRef.GetShaderResourceView() : intptr_t();
				std::string pickerLabel = "Preview Skybox | ";
				if (skyboxAssetRep != nullptr)
					pickerLabel.append(skyboxAssetRep->Name);

				// TODO.NW: Filter away cubemaps with Axel's filtering
				SAssetPickResult result = GUI::AssetPicker(pickerLabel.c_str(), "Preview Skybox", assetPickerThumbnail, "Assets/Textures/Cubemaps", 4, Manager->GetAssetInspectFunction());

				if (result.State == EAssetPickerState::AssetPicked)
				{
					CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
					assetRegistry->UnrequestAsset(PreviewSkylightAssetRef, MaterialToolRenderID);
					skyboxAssetRep = Manager->GetAssetRepFromDirEntry(result.PickedEntry).get();
					PreviewSkylightAssetRef = SAssetReference(skyboxAssetRep->DirectoryEntry.path().string());
					PreviewSkylight = assetRegistry->RequestAssetData<STextureAsset>(PreviewSkylightAssetRef, MaterialToolRenderID);
				}
				
				GUI::PushItemWidth(78.0f);
				GUI::DragFloat("Preview Light Intensity", PreviewLightIntensity, 0.01f);
				GUI::PopItemWidth();
				GUI::EndChild();
				
				GUI::SameLine();

				GUI::BeginChild("Light", settingsPropertySize);
				GUI::PushItemWidth(78.0f);
				GUI::ColorPicker3("Preview Light Color", PreviewLightColor);
				GUI::PopItemWidth();
				GUI::EndChild();
			}

			GUI::EndChild();
		}

		{ // Properties
			GUI::BeginChild("Properties", SVector2<F32>(212.0f, 0.0f), { EChildFlag::Borders, EChildFlag::ResizeX });
			GUI::Text("Properties");
			GUI::Separator();

			auto inspect = [this](SRuntimeGraphicsMaterialProperty& property, const std::string& label) 
				{
					constexpr F32 materialPropertyWidth = 32.0f;
					const F32 thumbnailPadding = 4.0f;
					const F32 cellWidth = materialPropertyWidth + thumbnailPadding;
					const F32 panelWidth = 256.0f;
					const I32 columnCount = static_cast<I32>(panelWidth / cellWidth);

					GUI::PushID(label.c_str());

					GUI::TextDisabled(label.c_str());
					GUI::PushItemWidth(72.0f);
					GUI::DragFloat("Constant Value", property.ConstantValue, 0.01f, -1.0f, 1.0f, "%.2f");
					GUI::PopItemWidth();

					if (property.ConstantValue < 0.0f)
					{
						I32 channelIndex = UMath::Clamp(STATIC_I32(property.TextureChannelIndex), 0, 3);
						GUI::PushItemWidth(72.0f);
						GUI::InputInt("Texture Channel", channelIndex, 1);
						GUI::PopItemWidth();
						property.TextureChannelIndex = STATIC_F32(channelIndex);

						std::string assetPath = GEngine::GetAssetRegistry()->GetAssetDatabaseEntry(property.TextureUID);
						auto assetRep = Manager->GetAssetRepFromName(UGeneralUtils::ExtractFileBaseNameFromPath(assetPath)).get();

						intptr_t assetPickerThumbnail = assetRep != nullptr ? (intptr_t)assetRep->TextureRef.GetShaderResourceView() : intptr_t();
						std::string pickerLabel = "";
						if (assetRep != nullptr)
							pickerLabel.append(assetRep->Name);
						
						// TODO.NW: Filter away cubemaps with Axel's filtering
						SAssetPickResult result = GUI::AssetPicker(pickerLabel.c_str(), "Texture", assetPickerThumbnail, "Assets/Textures", columnCount, Manager->GetAssetInspectFunction(), SVector2<F32>(materialPropertyWidth));

						if (result.State == EAssetPickerState::AssetPicked)
						{
							assetRep = Manager->GetAssetRepFromDirEntry(result.PickedEntry).get();
							property.TextureUID = SAssetReference(assetRep->DirectoryEntry.path().string()).UID;
						}
					}

					GUI::PopID();
					GUI::Separator();
				};

			inspect(MaterialData.AlbedoR, "Albedo R");
			inspect(MaterialData.AlbedoG, "Albedo G");
			inspect(MaterialData.AlbedoB, "Albedo B");
			inspect(MaterialData.AlbedoA, "Albedo A");
			inspect(MaterialData.NormalX, "Normal X");
			inspect(MaterialData.NormalY, "Normal Y");
			
			GUI::Checkbox("Recreate Normal Z In Shader", MaterialData.RecreateNormalZ);
			if (!MaterialData.RecreateNormalZ)
				inspect(MaterialData.NormalZ, "Normal Z");
			else
				GUI::Separator();
			
			inspect(MaterialData.AmbientOcclusion, "Ambient Occlusion");
			inspect(MaterialData.Metalness, "Metalness");
			inspect(MaterialData.Roughness, "Roughness");
			inspect(MaterialData.Emissive, "Emissive");

			GUI::EndChild();
			GUI::SameLine();
		}

		{ // Editor
			RotationInput.X = UMath::WrapAngle(RotationInput.X); // Pitch
			RotationInput.Y = UMath::WrapAngle(RotationInput.Y); // Yaw

			RenderMaterial();

			MaterialRender = Manager->GetRenderManager()->GetRenderTargetTexture(MaterialToolRenderID);
			if (MaterialRender != nullptr)
			{
				GUI::Image((intptr_t)MaterialRender->GetShaderResourceView(), SVector2<F32>(512.0f));
				IsHoveringViewport = GUI::IsItemHovered();
			}
			else
				IsHoveringViewport = false;
		}

		GUI::End();
	}

	void CMaterialTool::OnDisable()
	{
		CloseMaterial();
	}

	void CMaterialTool::OpenMaterial(SEditorAssetRepresentation* asset)
	{
		CurrentMaterial = asset;

		// TODO.NW: Want nicer interface for opening assets and closing them when saving

		const std::string filePath = asset->DirectoryEntry.path().string();
		const U64 fileSize = UFileSystem::GetFileSize(filePath);
		char* data = new char[fileSize];
		UFileSystem::Deserialize(filePath, data, STATIC_U32(fileSize));
		SMaterialAssetFileHeader assetFile;
		assetFile.Deserialize(data);
		delete[] data;

		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		assetRegistry->RequestAsset(SAssetReference(filePath), MaterialToolRenderID);
		PreviewSkylight = assetRegistry->RequestAssetData<STextureAsset>(PreviewSkylightAssetRef, MaterialToolRenderID);

		MaterialData = SGraphicsMaterialAsset(assetFile).Material;
		Manager->GetRenderManager()->RequestRenderView(MaterialToolRenderID);

		CurrentZoom = StartingZoom;
		RotationInput = SVector();
		SetEnabled(true);
	}

	void CMaterialTool::CloseMaterial()
	{
		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		if (CurrentMaterial)
			assetRegistry->UnrequestAsset(SAssetReference(CurrentMaterial->DirectoryEntry.path().string()), CAssetRegistry::EditorManagerRequestID);

		assetRegistry->UnrequestAsset(PreviewSkylightAssetRef, MaterialToolRenderID);

		CurrentMaterial = nullptr;
		MaterialData = SEngineGraphicsMaterial();
		Manager->GetRenderManager()->UnrequestRenderView(MaterialToolRenderID);

		SetEnabled(false);
	}

	void CMaterialTool::OnZoomInput(const SInputAxisPayload payload)
	{
		if (!IsHoveringViewport)
			return;

		const F32 zoomSpeed = -20.0f * GTime::Dt();
		CurrentZoom += payload.AxisValue * zoomSpeed;
		CurrentZoom = UMath::Clamp(CurrentZoom, 0.6f, 2.0f);
	}

	void CMaterialTool::HandleAxisInput(const SInputAxisPayload payload)
	{
		if (!IsOrbiting)
			return;

		constexpr F32 rotationSpeed = 90.0f;
		switch (payload.Event)
		{
		case EInputAxisEvent::MouseDeltaVertical:
			RotationInput.X +=  -payload.AxisValue * rotationSpeed * GTime::Dt();
			return;
		case EInputAxisEvent::MouseDeltaHorizontal:
			RotationInput.Y += payload.AxisValue * rotationSpeed * GTime::Dt();
			return;
		default:
			return;
		}
	}

	void CMaterialTool::ToggleFreeCam(const SInputActionPayload payload)
	{
		if (IsHoveringViewport && payload.IsHeld)
			IsOrbiting = true;
		else if (!payload.IsHeld)
			IsOrbiting = false;
	}

	void CMaterialTool::RenderMaterial()
	{
		CRenderManager* renderManager = Manager->GetRenderManager();

		if (PreviewSkylight == nullptr)
			return;

		{
			const F32 radius = 2.0f * CurrentZoom;
			SVector location = SVector(0.0f, 0.0f, radius);
			SMatrix camView = SMatrix::LookAtLH(location, SVector(), SVector::Up).FastInverse();
			SMatrix camProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), 1.0f, 0.01f, 10.0f);

			SRenderCommand command = SRenderCommand(ERenderCommandType::CameraDataStorage);
			command.Matrices.emplace_back(camView);
			command.Matrices.emplace_back(camProjection);
			renderManager->PushRenderCommand(command, MaterialToolRenderID);
		}

		{
			STransformComponent component;
			SMatrix objectMatrix = SMatrix::CreateRotationFromEuler(SVector(RotationInput.X, RotationInput.Y, 0.0f)).FastInverse();
			component.Transform.SetMatrix(objectMatrix);
			component.Owner = SEntity(MaterialToolRenderID);

			SDrawCallData data;
			data.IndexCount = STATIC_U32(GeometryPrimitives::Icosphere.Indices.size());
			data.VertexBufferIndex = STATIC_U8(EVertexBufferPrimitives::Icosphere);
			data.IndexBufferIndex = STATIC_U8(EIndexBufferPrimitives::Icosphere);
			data.VertexStrideIndex = 0;
			data.VertexOffsetIndex = 0;
			data.MaterialIndex = 0;

			SRenderCommand command;
			command.Type = ERenderCommandType::GBufferDataInstanced;
			command.U32s.push_back(MaterialToolPreviewAssetID);
			command.DrawCallData.emplace_back(data);
			command.Materials.push_back(MaterialData);
			command.MaterialRenderTextures.push_back(MaterialData.GetRenderTextures(MaterialToolRenderID));
			renderManager->AddStaticMeshToInstancedRenderList(MaterialToolPreviewAssetID, &component, MaterialToolRenderID);
			renderManager->PushRenderCommand(command, MaterialToolRenderID);
		}

		{
			const SVector4 directionalLightDirection = { 1.0f, 0.0f, 1.0f, 0.0f };
			const SVector4 directionalLightColor = { PreviewLightColor.AsVector(), PreviewLightIntensity };

			SShadowmapViewData shadowmapView;
			shadowmapView.ShadowProjectionMatrix = SMatrix::OrthographicLH(8.0f, 8.0f, -8.0f, 8.0f);

			SRenderCommand command;
			command.Type = ERenderCommandType::DeferredLightingDirectional;
			command.Vectors.push_back(directionalLightDirection);
			command.Colors.push_back(directionalLightColor);
			command.ShadowmapViews.push_back(shadowmapView);
			command.RenderTextures.push_back(PreviewSkylight->RenderTexture);
			renderManager->PushRenderCommand(command, MaterialToolRenderID);			
		}

		{
			SRenderCommand command;
			command.RenderTextures.push_back(PreviewSkylight->RenderTexture);
			command.Type = ERenderCommandType::Skybox;
			renderManager->PushRenderCommand(command, MaterialToolRenderID);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::PreLightingPass;
			renderManager->PushRenderCommand(command, MaterialToolRenderID);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::PostBaseLightingPass;
			renderManager->PushRenderCommand(command, MaterialToolRenderID);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::Bloom;
			renderManager->PushRenderCommand(command, MaterialToolRenderID);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::Tonemapping;
			renderManager->PushRenderCommand(command, MaterialToolRenderID);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::AntiAliasing;
			renderManager->PushRenderCommand(command, MaterialToolRenderID);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::GammaCorrection;
			renderManager->PushRenderCommand(command, MaterialToolRenderID);
		}
	}
}
