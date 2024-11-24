// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "ViewportWindow.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"
#include "Graphics/RenderManager.h"
#include "Graphics/FullscreenTexture.h"

#include <Havtorn/Utilities.h>

namespace ImGui
{
	CViewportWindow::CViewportWindow(const char* displayName, Havtorn::CEditorManager* manager)
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
		const Havtorn::SEditorLayout& layout = Manager->GetEditorLayout();

		const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + layout.ViewportPosition.X, mainViewport->WorkPos.y + layout.ViewportPosition.Y));
		ImGui::SetNextWindowSize(ImVec2(layout.ViewportSize.X, layout.ViewportSize.Y));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		ImTextureID playButtonID = (ImTextureID)(intptr_t)Manager->GetResourceManager()->GetEditorTexture(Havtorn::EEditorTexture::PlayIcon);
		ImTextureID pauseButtonID = (ImTextureID)(intptr_t)Manager->GetResourceManager()->GetEditorTexture(Havtorn::EEditorTexture::PauseIcon);
		ImTextureID stopButtonID = (ImTextureID)(intptr_t)Manager->GetResourceManager()->GetEditorTexture(Havtorn::EEditorTexture::StopIcon);

		if (ImGui::Begin(Name(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			ImVec2 buttonSize = { 16.0f, 16.0f };
			ImVec4* colors = ImGui::GetStyle().Colors;
			ImVec4 buttonInactiveColor = colors[ImGuiCol_Button];
			ImVec4 buttonActiveColor = colors[ImGuiCol_ButtonActive];
			ImVec4 buttonHoveredColor = colors[ImGuiCol_ButtonHovered];
			ImVec2 uv0 = { 0.0f, 0.0f };
			ImVec2 uv1 = { 1.0f, 1.0f };

			// TODO.NR: Make an abstraction for what's happening inside and to these button blocks
			// TODO.NR: Make util button function based on ImGui::ImageButtonEx that can fill the whole rect (not only image background) with the color we choose

			ImGui::SameLine(layout.ViewportSize.X * 0.5f - 8.0f - 32.0f);
			ImVec4 playButtonColor = IsPlayButtonEngaged ? buttonActiveColor : IsPlayButtonHovered ? buttonHoveredColor : buttonInactiveColor;
			if (ImGui::ImageButton("PlayButton", playButtonID, buttonSize, uv0, uv1, playButtonColor))
			{
				if (Havtorn::GEngine::GetWorld()->BeginPlay())
				{
					IsPlayButtonEngaged = true;
					IsPauseButtonEngaged = false;
				}
			}
			IsPlayButtonHovered = IsItemHovered();

			ImGui::SameLine(layout.ViewportSize.X * 0.5f - 8.0f);
			ImVec4 pauseButtonColor = IsPauseButtonEngaged ? buttonActiveColor : IsPauseButtonHovered ? buttonHoveredColor : buttonInactiveColor;
			if (ImGui::ImageButton("PauseButton", pauseButtonID, buttonSize, uv0, uv1, pauseButtonColor))
			{
				if (Havtorn::GEngine::GetWorld()->PausePlay())
				{
					IsPlayButtonEngaged = false;
					IsPauseButtonEngaged = true;
				}
			}
			IsPauseButtonHovered = IsItemHovered();

			ImGui::SameLine(layout.ViewportSize.X * 0.5f - 8.0f + 32.0f);
			if (ImGui::ImageButton("StopButton", stopButtonID, buttonSize))
			{
				if (Havtorn::GEngine::GetWorld()->StopPlay())
				{
					IsPlayButtonEngaged = false;
					IsPauseButtonEngaged = false;					
				}
			}

			ImGuiStyle style = ImGui::GetStyle();
			ImGui::SameLine(layout.ViewportSize.X * 0.5f - 8.0f + 64.0f);
			std::string playDimensionLabel = Havtorn::GEngine::GetWorld()->GetWorldPlayDimensions() == Havtorn::EWorldPlayDimensions::World3D ? "3D" : "2D";
			if (ImGui::Button(playDimensionLabel.c_str(), buttonSize + style.FramePadding * 2.0f))
			{
				Havtorn::GEngine::GetWorld()->ToggleWorldPlayDimensions();
			}

			RenderedSceneTextureReference = &(Manager->GetRenderManager()->GetRenderedSceneTexture());
			
			if (RenderedSceneTextureReference)
			{
				ImVec2 vMin = ImGui::GetWindowContentRegionMin();
				ImVec2 vMax = ImGui::GetWindowContentRegionMax();

				Havtorn::F32 width = static_cast<Havtorn::F32>(vMax.x - vMin.x);
				Havtorn::F32 height = static_cast<Havtorn::F32>(vMax.y - vMin.y - ViewportMenuHeight - 4.0f);

				ImVec2 windowPos = ImVec2(mainViewport->WorkPos.x + layout.ViewportPosition.X, mainViewport->WorkPos.y + layout.ViewportPosition.Y);
				windowPos.y += ViewportMenuHeight - 4.0f;
				RenderedScenePosition.X = windowPos.x;
				RenderedScenePosition.Y = windowPos.y;
				RenderedSceneDimensions = { width, height };

				ImGui::Image((ImTextureID)(intptr_t)RenderedSceneTextureReference->GetShaderResourceView(), ImVec2(width, height));
			}
		
			CurrentDrawList = ImGui::GetWindowDrawList();
		}

		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::End();
	}

	void CViewportWindow::OnDisable()
	{
	}
	
	const Havtorn::SVector2<Havtorn::F32> CViewportWindow::GetRenderedSceneDimensions() const
	{
		return RenderedSceneDimensions;
	}
	
	const Havtorn::SVector2<Havtorn::F32> CViewportWindow::GetRenderedScenePosition() const
	{
		return RenderedScenePosition;
	}
	
	ImDrawList* CViewportWindow::GetCurrentDrawList() const
	{
		return CurrentDrawList;
	}
}