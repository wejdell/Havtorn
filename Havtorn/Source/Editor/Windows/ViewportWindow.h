// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

#include <GUI.h>

struct ID3D11Texture2D;

namespace Havtorn
{
	class CRenderTexture;

	class CViewportWindow : public CWindow
	{
	public:
		CViewportWindow(const char* displayName, CEditorManager* manager);
		~CViewportWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

		const SVector2<F32> GetRenderedSceneDimensions() const;
		const SVector2<F32> GetRenderedScenePosition() const;

		GUI::SGuiDrawList* GetCurrentDrawList() const;

	private:
		const CRenderTexture* RenderedSceneTextureReference;
		GUI::SGuiDrawList* CurrentDrawList = nullptr;
		F32 ViewportMenuHeight = 16.0f;
		SVector2<F32> RenderedSceneDimensions = SVector2<F32>::Zero;
		SVector2<F32> RenderedScenePosition = SVector2<F32>::Zero;

		// TODO.NR: Make an abstraction for what's happening inside and to the play button blocks
		bool IsPlayButtonEngaged = false;
		bool IsPauseButtonEngaged = false;
		bool IsPlayButtonHovered = false;
		bool IsPauseButtonHovered = false;
	};
}
