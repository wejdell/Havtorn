// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "DockSpaceWindow.h"

#include "EditorManager.h"

#include <GUI.h>

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
		SEditorLayout& layout = Manager->GetEditorLayout();

		const SVector2<F32> layoutPosition = SVector2<F32>(layout.DockSpacePosition.X, layout.DockSpacePosition.Y);
		const SVector2<F32> layoutSize = SVector2<F32>(layout.DockSpaceSize.X, layout.DockSpaceSize.Y);
		GUI::SetNextWindowPos(layoutPosition);
		GUI::SetNextWindowSize(layoutSize);

		// NW: Remove padding and similar around the docking area
		GUI::PushStyleVar(EStyleVar::WindowRounding, 0.0f);
		GUI::PushStyleVar(EStyleVar::WindowBorderSize, 0.0f);
		GUI::PushStyleVar(EStyleVar::WindowPadding, SVector2<F32>(0.0f, 0.0f));

		GUI::Begin(Name(), nullptr, { EWindowFlag::NoScrollbar, EWindowFlag::NoBackground, EWindowFlag::NoTitleBar, EWindowFlag::NoMove, EWindowFlag::NoResize, EWindowFlag::NoCollapse, EWindowFlag::NoBringToFrontOnFocus });
		GUI::PopStyleVar(3);

		if (GUI::IsDockingEnabled()) 
		{
			I32 dockSpaceID = GUI::GetID(Name());
			GUI::DockSpace(dockSpaceID, SVector2<F32>(0.0f, 0.0f), EDockNodeFlag::PassthruCentralNode);

			static bool hasInitialized = true;
			if (hasInitialized)
			{
				hasInitialized = false;
				// Clear out existing layout
				GUI::DockBuilderRemoveNode(dockSpaceID);
				// Add empty node
				GUI::DockBuilderAddNode(dockSpaceID, { EDockNodeFlag::PassthruCentralNode, EDockNodeFlag::DockSpace });
				// Main node should cover entire window
				GUI::DockBuilderSetNodeSize(dockSpaceID, GUI::GetCurrentWindowSize());
				// get id of main dock space area
				I32 dockSpaceIDCopy = dockSpaceID;

				// NW: Keep this in case we'd like to use it over an element in the asset browser
				// Create a dock node for the right docked window
				//ImGuiID sideViewID = GUI::DockBuilderSplitNode(dockSpaceIDCopy, ImGuiDir_Left, 0.25f, nullptr, &dockSpaceIDCopy);
				//GUI::DockBuilderDockWindow("Asset Browser Folder View", sideViewID);

				GUI::DockBuilderDockWindow("Asset Browser", dockSpaceIDCopy);
				GUI::DockBuilderDockWindow("Output Log", dockSpaceIDCopy);
				GUI::DockBuilderFinish(dockSpaceID);
			}
		}

		const SVector2<F32> newPosition = GUI::GetWindowPos();
		const SVector2<F32> newSize = GUI::GetWindowSize();
		if (layoutPosition != newPosition || layoutPosition != newSize)
		{
			layout.DockSpacePosition.X = STATIC_I16(newPosition.X);
			layout.DockSpacePosition.Y = STATIC_I16(newPosition.Y);
			layout.DockSpaceSize.X = STATIC_U16(newSize.X);
			layout.DockSpaceSize.Y = STATIC_U16(newSize.Y);
			layout.DockSpaceChanged = true;
		}

		GUI::End();
	}

	void CDockSpaceWindow::OnDisable()
	{
	}
}
