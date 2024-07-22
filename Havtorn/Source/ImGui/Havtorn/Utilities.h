// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core/imgui.h"
#include <string>

namespace ImGui
{
	struct UUtils
	{
		static const float SliderSpeed;
		static const float TexturePreviewSizeX;
		static const float TexturePreviewSizeY;
		static const float DummySizeX;
		static const float DummySizeY;
		static const float ThumbnailPadding;
		static const float PanelWidth;

		static const std::string SelectTextureModalName;

		static const ImVec2 TexturePreviewSize;
		static const ImVec2 DummySize;
		static const ImVec2 ModalWindowPivot;

		//void TryOpenModalTableWindow(const std::string& identifier, const std::string& searchPath);
	};
}