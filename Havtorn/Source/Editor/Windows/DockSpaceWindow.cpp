// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "DockSpaceWindow.h"

#include "EditorManager.h"

namespace Havtorn
{
	CDockSpaceWindow::CDockSpaceWindow(const char* displayName, CEditorManager* manager)
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
		const SEditorLayout& layout = Manager->GetEditorLayout();
		const SGuiViewport* mainViewport = GUI::GetMainViewport();

		GUI::SetNextWindowPos(SVector2<F32>(mainViewport->WorkPos.x + layout.AssetBrowserPosition.X, mainViewport->WorkPos.y + layout.AssetBrowserPosition.Y));
		GUI::SetNextWindowSize(SVector2<F32>(layout.AssetBrowserSize.X, layout.AssetBrowserSize.Y));

		// NR: Remove padding and similar around the docking area
		GUI::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		GUI::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		GUI::PushStyleVar(ImGuiStyleVar_WindowPadding, SVector2<F32>(0.0f, 0.0f));

		GUI::Begin(Name(), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
		GUI::PopStyleVar(3);

		SGuiIO& io = GUI::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) 
		{
			SGuiID dockSpaceID = GUI::GetID(Name());
			GUI::DockSpace(dockSpaceID, SVector2<F32>(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

			static bool hasInitialized = true;
			if (hasInitialized)
			{
				hasInitialized = false;
				// Clear out existing layout
				GUI::DockBuilderRemoveNode(dockSpaceID);
				// Add empty node
				GUI::DockBuilderAddNode(dockSpaceID, ImGuiDockNodeFlags(1032)/*ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace*/);
				// Main node should cover entire window
				GUI::DockBuilderSetNodeSize(dockSpaceID, GUI::GetWindowSize());
				// get id of main dock space area
				ImGuiID dockSpaceIDCopy = dockSpaceID;

				// NR: Keep this in case we'd like to use it over an element in the asset browser
				// Create a dock node for the right docked window
				//ImGuiID sideViewID = GUI::DockBuilderSplitNode(dockSpaceIDCopy, ImGuiDir_Left, 0.25f, nullptr, &dockSpaceIDCopy);
				//GUI::DockBuilderDockWindow("Asset Browser Folder View", sideViewID);

				GUI::DockBuilderDockWindow("Asset Browser", dockSpaceIDCopy);
				GUI::DockBuilderDockWindow("Output Log", dockSpaceIDCopy);
				GUI::DockBuilderFinish(dockSpaceID);
			}
		}

		GUI::End();
	}

	void CDockSpaceWindow::OnDisable()
	{
	}
}
