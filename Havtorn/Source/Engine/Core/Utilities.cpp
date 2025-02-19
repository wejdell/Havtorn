// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "Utilities.h"

#include <filesystem>
#include <imgui/imgui.h>

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

	//const ImVec2 UUtils::TexturePreviewSize = { UUtils::TexturePreviewSizeX, UUtils::TexturePreviewSizeY };
	//const ImVec2 UUtils::DummySize = { UUtils::DummySizeX, UUtils::DummySizeY };
	//const ImVec2 UUtils::ModalWindowPivot = { 0.5f, 0.5f };

	bool UUtils::TryOpenComponentView(const std::string& componentViewName)
	{
		return ImGui::CollapsingHeader(componentViewName.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
	}
}
