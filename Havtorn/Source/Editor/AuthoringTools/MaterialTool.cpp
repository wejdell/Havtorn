// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"

#include <FileSystem.h>
#include <Graphics/RenderManager.h>
#include <Input/InputMapper.h>
#include <Input/InputTypes.h>
#include <Timer.h>
#include <Scene/AssetRegistry.h>

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

		{ // Menu Bar
			GUI::BeginChild("MaterialOptions", SVector2<F32>(0.0f, 30.0f));
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
			GUI::EndChild();
		}

		{ // Properties
			GUI::BeginChild("Properties", SVector2<F32>(150.0f, 0.0f), { EChildFlag::Borders, EChildFlag::ResizeX });
			GUI::Text("Properties");
			GUI::Separator();

			auto inspect = [this](SRuntimeGraphicsMaterialProperty& property, const std::string& label) 
				{
					F32 thumbnailPadding = 4.0f;
					F32 cellWidth = GUI::TexturePreviewSizeX * 0.75f + thumbnailPadding;
					F32 panelWidth = 256.0f;
					I32 columnCount = static_cast<I32>(panelWidth / cellWidth);

					GUI::PushID(label.c_str());

					GUI::TextDisabled(label.c_str());
					GUI::DragFloat("Constant Value", property.ConstantValue, 0.01f, -1.0f, 1.0f, "%.2f");
					
					if (property.ConstantValue < 0.0f)
					{
						std::string assetPath = GEngine::GetAssetRegistry()->GetAssetDatabaseEntry(property.TextureUID);
						auto assetRep = Manager->GetAssetRepFromName(UGeneralUtils::ExtractFileBaseNameFromPath(assetPath)).get();

						intptr_t assetPickerThumbnail = assetRep != nullptr ? (intptr_t)assetRep->TextureRef.GetShaderResourceView() : intptr_t();
						std::string pickerLabel = "";
						if (assetRep != nullptr)
							pickerLabel.append(assetRep->Name);
						
						// TODO.NW: Filter away cubemaps with Axel's filtering
						SAssetPickResult result = GUI::AssetPicker(pickerLabel.c_str(), "Texture", assetPickerThumbnail, "Assets/Textures", columnCount, Manager->GetAssetInspectFunction());

						if (result.State == EAssetPickerState::AssetPicked)
						{
							assetRep = Manager->GetAssetRepFromDirEntry(result.PickedEntry).get();
							property.TextureUID = SAssetReference(assetRep->DirectoryEntry.path().string()).UID;
						}
						I32 channelIndex = UMath::Clamp(STATIC_I32(property.TextureChannelIndex), 0, 3);
						GUI::InputInt("Texture Channel Index", channelIndex, 1);
						property.TextureChannelIndex = STATIC_F32(channelIndex);
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

			Manager->GetRenderManager()->RenderMaterialTexture(MaterialRender, MaterialData, RotationInput, CurrentZoom);
			GUI::Image((intptr_t)MaterialRender.GetShaderResourceView(), SVector2<F32>(512.0f));
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

		GEngine::GetAssetRegistry()->RequestAsset(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);

		MaterialData = SGraphicsMaterialAsset(assetFile).Material;
		MaterialRender = Manager->GetRenderManager()->RenderMaterialAssetTexture(filePath);

		CurrentZoom = StartingZoom;
		RotationInput = SVector();
		SetEnabled(true);
	}

	void CMaterialTool::CloseMaterial()
	{
		if (CurrentMaterial)
			GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(CurrentMaterial->DirectoryEntry.path().string()), CAssetRegistry::EditorManagerRequestID);

		CurrentMaterial = nullptr;
		MaterialData = SEngineGraphicsMaterial();
		MaterialRender.Release();
		SetEnabled(false);
	}

	void CMaterialTool::OnZoomInput(const SInputAxisPayload payload)
	{
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
		IsOrbiting = payload.IsHeld;
	}
}
