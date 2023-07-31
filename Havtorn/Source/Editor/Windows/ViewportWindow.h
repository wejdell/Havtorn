// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

#include <Core/imgui.h>

struct ID3D11Texture2D;

namespace Havtorn
{
	class CFullscreenTexture;
}

namespace ImGui
{
	class CViewportWindow : public CWindow
	{
	public:
		CViewportWindow(const char* displayName, Havtorn::CEditorManager* manager);
		~CViewportWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

		const Havtorn::SVector2<Havtorn::F32> GetRenderedSceneDimensions() const;
		const Havtorn::SVector2<Havtorn::F32> GetRenderedScenePosition() const;

		ImDrawList* GetCurrentDrawList() const;

	private:
		const Havtorn::CFullscreenTexture* RenderedSceneTextureReference;
		ImDrawList* CurrentDrawList = nullptr;
		Havtorn::F32 ViewportMenuHeight = 16.0f;
		Havtorn::SVector2<Havtorn::F32> RenderedSceneDimensions = Havtorn::SVector2<Havtorn::F32>::Zero;
		Havtorn::SVector2<Havtorn::F32> RenderedScenePosition = Havtorn::SVector2<Havtorn::F32>::Zero;
	};
}
