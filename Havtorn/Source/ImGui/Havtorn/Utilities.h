// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

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

		static const std::string SelectTextureModalName;
	};

	const float UUtils::SliderSpeed = 0.1f;
	const float UUtils::TexturePreviewSizeX = 64.f;
	const float UUtils::TexturePreviewSizeY = 64.f;
	const float UUtils::DummySizeX = 0.0f;
	const float UUtils::DummySizeY = 0.5f;
	const float UUtils::ThumbnailPadding = 4.0f;

	const std::string UUtils::SelectTextureModalName = "Select Texture Asset";
}