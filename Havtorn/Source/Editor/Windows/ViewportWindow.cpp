// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ViewportWindow.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderingPrimitives/FullscreenTexture.h"

namespace Havtorn
{
	CViewportWindow::CViewportWindow(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager)
		, RenderedSceneTextureReference(nullptr)
	{
	}

	CViewportWindow::~CViewportWindow()
	{
	}

	void CViewportWindow::OnEnable()
	{
	}

	void CViewportWindow::OnInspectorGUI()
	{
		const SEditorLayout& layout = Manager->GetEditorLayout();

		const GUI::SGuiMainViewport* mainViewport = GUI::GetMainViewport();
		GUI::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + layout.ViewportPosition.X, mainViewport->WorkPos.y + layout.ViewportPosition.Y));
		GUI::SetNextWindowSize(ImVec2(layout.ViewportSize.X, layout.ViewportSize.Y));
		GUI::PushStyleVar(GUIStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		GUI::PushStyleVar(GUIStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		SGuiTextureID playButtonID = (SGuiTextureID)(intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::PlayIcon);
		SGuiTextureID pauseButtonID = (SGuiTextureID)(intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::PauseIcon);
		SGuiTextureID stopButtonID = (SGuiTextureID)(intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::StopIcon);

		if (GUI::Begin(Name(), nullptr, GUIWindowFlags_NoMove | GUIWindowFlags_NoResize | GUIWindowFlags_NoCollapse | GUIWindowFlags_NoBringToFrontOnFocus))
		{
			SVector2<F32> buttonSize = { 16.0f, 16.0f };
			SColor* colors = GUI::GetStyle().Colors;
			SColor buttonInactiveColor = colors[GUICol_Button];
			SColor buttonActiveColor = colors[GUICol_ButtonActive];
			SColor buttonHoveredColor = colors[GUICol_ButtonHovered];
			SVector2<F32> uv0 = { 0.0f, 0.0f };
			SVector2<F32> uv1 = { 1.0f, 1.0f };

			// TODO.NR: Make an abstraction for what's happening inside and to these button blocks
			// TODO.NR: Make util button function based on GUI::ImageButtonEx that can fill the whole rect (not only image background) with the color we choose


			GUI::SameLine(layout.ViewportSize.X * 0.5f - 8.0f - 32.0f);
			SColor playButtonColor = IsPlayButtonEngaged ? buttonActiveColor : IsPlayButtonHovered ? buttonHoveredColor : buttonInactiveColor;
			if (GUI::ImageButton("PlayButton", playButtonID, buttonSize, uv0, uv1, playButtonColor))
			{
				if (GEngine::GetWorld()->BeginPlay())
				{
					IsPlayButtonEngaged = true;
					IsPauseButtonEngaged = false;
				}
			}
			IsPlayButtonHovered = GUI::IsItemHovered();

			GUI::SameLine(layout.ViewportSize.X * 0.5f - 8.0f);
			SColor pauseButtonColor = IsPauseButtonEngaged ? buttonActiveColor : IsPauseButtonHovered ? buttonHoveredColor : buttonInactiveColor;
			if (GUI::ImageButton("PauseButton", pauseButtonID, buttonSize, uv0, uv1, pauseButtonColor))
			{
				if (GEngine::GetWorld()->PausePlay())
				{
					IsPlayButtonEngaged = false;
					IsPauseButtonEngaged = true;
				}
			}
			IsPauseButtonHovered = GUI::IsItemHovered();

			GUI::SameLine(layout.ViewportSize.X * 0.5f - 8.0f + 32.0f);
			if (GUI::ImageButton("StopButton", stopButtonID, buttonSize))
			{
				if (GEngine::GetWorld()->StopPlay())
				{
					IsPlayButtonEngaged = false;
					IsPauseButtonEngaged = false;					
				}
			}

			GUI::SGuiStyle style = GUI::GetStyle();
			GUI::SameLine(layout.ViewportSize.X * 0.5f - 8.0f + 64.0f);
			std::string playDimensionLabel = GEngine::GetWorld()->GetWorldPlayDimensions() == EWorldPlayDimensions::World3D ? "3D" : "2D";
			if (GUI::Button(playDimensionLabel.c_str(), buttonSize + style.FramePadding * 2.0f))
			{
				GEngine::GetWorld()->ToggleWorldPlayDimensions();
			}

			RenderedSceneTextureReference = &(Manager->GetRenderManager()->GetRenderedSceneTexture());
			
			if (RenderedSceneTextureReference)
			{
				SVector2<F32> vMin = GUI::GetWindowContentRegionMin();
				SVector2<F32> vMax = GUI::GetWindowContentRegionMax();

				F32 width = static_cast<F32>(vMax.X - vMin.X);
				F32 height = static_cast<F32>(vMax.Y - vMin.Y - ViewportMenuHeight - 4.0f);

				SVector2<F32> windowPos = SVector2<F32>(mainViewport->WorkPos.x + layout.ViewportPosition.X, mainViewport->WorkPos.y + layout.ViewportPosition.Y);
				windowPos.Y += ViewportMenuHeight - 4.0f;
				RenderedScenePosition.X = windowPos.X;
				RenderedScenePosition.Y = windowPos.Y;
				RenderedSceneDimensions = { width, height };

				GUI::Image((SGuiTextureID)(intptr_t)RenderedSceneTextureReference->GetShaderResourceView(), ImVec2(width, height));
			}
		
			CurrentDrawList = GUI::GetWindowDrawList();
		}

		GUI::PopStyleVar();
		GUI::PopStyleVar();
		GUI::End();
	}

	void CViewportWindow::OnDisable()
	{
	}
	
	const SVector2<F32> CViewportWindow::GetRenderedSceneDimensions() const
	{
		return RenderedSceneDimensions;
	}
	
	const SVector2<F32> CViewportWindow::GetRenderedScenePosition() const
	{
		return RenderedScenePosition;
	}
	
	GUI::SGuiDrawList* CViewportWindow::GetCurrentDrawList() const
	{
		return CurrentDrawList;
	}
}