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

	bool UUtils::TryOpenComponentView(const std::string& componentViewName)
	{
		return ImGui::CollapsingHeader(componentViewName.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
	}

	// NR: Just as with viewing components, components know how to add themselves and how to remove them. 
	// Figure out an abstraction that holds all of this. It should be easy to extend components with specific knowledge of them this way.
	// Just a lot of boilerplate. Try to introduce this in base class? Should probably include sequencer node behavior as well
	//TryAddComponent()

	//TryRemoveComponent()
}
