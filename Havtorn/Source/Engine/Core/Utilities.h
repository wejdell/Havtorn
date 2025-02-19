// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

//#include "imgui.h"
//#include <imgui/imgui.h>
#include <string>

namespace ImGui
{
	struct UUtils
	{
		HAVTORN_API static const float SliderSpeed;
		HAVTORN_API static const float TexturePreviewSizeX;
		HAVTORN_API static const float TexturePreviewSizeY;
		HAVTORN_API static const float DummySizeX;
		HAVTORN_API static const float DummySizeY;
		HAVTORN_API static const float ThumbnailPadding;
		HAVTORN_API static const float PanelWidth;

		HAVTORN_API static const std::string SelectTextureModalName;

		//HAVTORN_API static const ImVec2 TexturePreviewSize;
		//HAVTORN_API static const ImVec2 DummySize;
		//HAVTORN_API static const ImVec2 ModalWindowPivot;

		HAVTORN_API static bool TryOpenComponentView(const std::string& componentViewName);
	};
}