// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "Utilities.h"

#include <filesystem>

namespace ImGui
{
	const float UUtils::SliderSpeed = 0.1f;
	const float UUtils::TexturePreviewSizeX = 64.f;
	const float UUtils::TexturePreviewSizeY = 64.f;
	const float UUtils::DummySizeX = 0.0f;
	const float UUtils::DummySizeY = 0.5f;
	const float UUtils::ThumbnailPadding = 4.0f;
	const float UUtils::PanelWidth = 256.0f;

	const std::string UUtils::SelectTextureModalName = "Select Texture Asset";

	const ImVec2 UUtils::TexturePreviewSize = { UUtils::TexturePreviewSizeX, UUtils::TexturePreviewSizeY };
	const ImVec2 UUtils::DummySize = { UUtils::DummySizeX, UUtils::DummySizeY };
	const ImVec2 UUtils::ModalWindowPivot = { 0.5f, 0.5f };

	//void UUtils::TryOpenModalTableWindow(const std::string& identifier, const std::string& searchPath)
	//{
	//	const std::string modalName = identifier + "ModalName";
	//	if (!ImGui::BeginPopupModal(modalName.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	//		return;

	//	float cellWidth = UUtils::TexturePreviewSizeX * 0.75f + UUtils::ThumbnailPadding;
	//	int columnCount = static_cast<int>(UUtils::PanelWidth / cellWidth);
	//	unsigned id = 0;

	//	const std::string tableName = identifier + "TableName";
	//	if (ImGui::BeginTable(tableName.c_str(), columnCount))
	//	{
	//		for (auto& entry : std::filesystem::recursive_directory_iterator(searchPath))
	//		{
	//			if (entry.is_directory())
	//				continue;

	//			auto& assetRep = Manager->GetAssetRepFromDirEntry(entry);

	//			ImGui::TableNextColumn();
	//			ImGui::PushID(id++);

	//			if (ImGui::ImageButton(assetRep->TextureRef, { UUtils::TexturePreviewSizeX * 0.75f, UUtils::TexturePreviewSizeY * 0.75f }))
	//			{

	//				Manager->GetRenderManager()->TryReplaceMaterialOnComponent(assetRep->DirectoryEntry.path().string(), materialIndex, materialComponentToChange);

	//				ImGui::CloseCurrentPopup();
	//			}

	//			ImGui::Text(assetRep->Name.c_str());
	//			ImGui::PopID();
	//		}

	//		ImGui::EndTable();
	//	}

	//	if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
	//		ImGui::CloseCurrentPopup();

	//	ImGui::EndPopup();
	//}
}
