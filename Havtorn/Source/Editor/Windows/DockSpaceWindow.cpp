// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "DockSpaceWindow.h"

#include "EditorManager.h"

#include <Core/imgui.h>
#include <Core/imgui_internal.h>

namespace ImGui
{
	CDockSpaceWindow::CDockSpaceWindow(const char* displayName, Havtorn::CEditorManager* manager)
		: CWindow(displayName, manager)
	{}

	CDockSpaceWindow::~CDockSpaceWindow()
	{		
	}

	void CDockSpaceWindow::OnEnable()
	{
	}

	void CDockSpaceWindow::OnInspectorGUI()
	{
		const Havtorn::SEditorLayout& layout = Manager->GetEditorLayout();
		const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + layout.AssetBrowserPosition.X, mainViewport->WorkPos.y + layout.AssetBrowserPosition.Y));
		ImGui::SetNextWindowSize(ImVec2(layout.AssetBrowserSize.X, layout.AssetBrowserSize.Y));

		// NR: Remove padding and similar around the docking area
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin(Name(), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
		ImGui::PopStyleVar(3);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) 
		{
			ImGuiID dockSpaceID = ImGui::GetID(Name());
			ImGui::DockSpace(dockSpaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

			static bool hasInitialized = true;
			if (hasInitialized)
			{
				hasInitialized = false;
				// Clear out existing layout
				ImGui::DockBuilderRemoveNode(dockSpaceID);
				// Add empty node
				ImGui::DockBuilderAddNode(dockSpaceID, ImGuiDockNodeFlags(1032)/*ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace*/);
				// Main node should cover entire window
				ImGui::DockBuilderSetNodeSize(dockSpaceID, ImGui::GetWindowSize());
				// get id of main dock space area
				ImGuiID dockSpaceIDCopy = dockSpaceID;

				// NR: Keep this in case we'd like to use it over an element in the asset browser
				// Create a dock node for the right docked window
				//ImGuiID sideViewID = ImGui::DockBuilderSplitNode(dockSpaceIDCopy, ImGuiDir_Left, 0.25f, nullptr, &dockSpaceIDCopy);
				//ImGui::DockBuilderDockWindow("Asset Browser Folder View", sideViewID);

				ImGui::DockBuilderDockWindow("Asset Browser", dockSpaceIDCopy);
				ImGui::DockBuilderDockWindow("Output Log", dockSpaceIDCopy);
				ImGui::DockBuilderFinish(dockSpaceID);
			}
		}

		ImGui::End();
	}

	void CDockSpaceWindow::OnDisable()
	{
	}
}
