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

		SVector2<F32> viewportWorkPos = GUI::GetViewportWorkPos();
		GUI::SetNextWindowPos(SVector2<F32>(viewportWorkPos.X + layout.ViewportPosition.X, viewportWorkPos.Y + layout.ViewportPosition.Y));
		GUI::SetNextWindowSize(SVector2<F32>(layout.ViewportSize.X, layout.ViewportSize.Y));
		GUI::PushStyleVar(EStyleVar::WindowPadding, SVector2<F32>(0.0f));
		GUI::PushStyleVar(EStyleVar::ItemSpacing, SVector2<F32>(0.0f));

		intptr_t playButtonID = (intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::PlayIcon);
		intptr_t pauseButtonID = (intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::PauseIcon);
		intptr_t stopButtonID = (intptr_t)Manager->GetResourceManager()->GetEditorTexture(EEditorTexture::StopIcon);

		if (GUI::Begin(Name(), nullptr, { EWindowFlag::NoMove, EWindowFlag::NoResize, EWindowFlag::NoCollapse, EWindowFlag::NoBringToFrontOnFocus }))
		{
			SVector2<F32> buttonSize = { 16.0f, 16.0f };
			const std::vector<SColor>& colors = GUI::GetStyleColors();
			SColor buttonInactiveColor = colors[STATIC_U64(EStyleColor::Button)];
			SColor buttonActiveColor = colors[STATIC_U64(EStyleColor::ButtonActive)];
			SColor buttonHoveredColor = colors[STATIC_U64(EStyleColor::ButtonHovered)];
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

			GUI::SameLine(layout.ViewportSize.X * 0.5f - 8.0f + 64.0f);
			std::string playDimensionLabel = GEngine::GetWorld()->GetWorldPlayDimensions() == EWorldPlayDimensions::World3D ? "3D" : "2D";
			if (GUI::Button(playDimensionLabel.c_str(), buttonSize + GUI::GetStyleVar(EStyleVar::FramePadding) * 2.0f))
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

				viewportWorkPos = GUI::GetViewportWorkPos();
				SVector2<F32> windowPos = SVector2<F32>(viewportWorkPos.X + layout.ViewportPosition.X, viewportWorkPos.Y + layout.ViewportPosition.Y);
				windowPos.Y += ViewportMenuHeight - 4.0f;
				RenderedScenePosition.X = windowPos.X;
				RenderedScenePosition.Y = windowPos.Y;
				RenderedSceneDimensions = { width, height };

				GUI::Image((intptr_t)RenderedSceneTextureReference->GetShaderResourceView(), SVector2<F32>(width, height));
			}
		
			GUI::SetGizmoDrawList();
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
}