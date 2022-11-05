// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "HierarchyWindow.h"
//#include <imgui.h>
#include "EditorManager.h"

#include "Engine.h"
#include "Scene/Scene.h"
#include "ECS/Entity.h"
#include "Core/MathTypes/EngineMath.h"
#include "Core/CoreTypes.h"

namespace ImGui
{
	CHierarchyWindow::CHierarchyWindow(const char* name, Havtorn::CEditorManager* manager)
		: CWindow(name, manager)
		, SelectedIndex(0)
	{
	}

	CHierarchyWindow::~CHierarchyWindow()
	{
	}

	void CHierarchyWindow::OnEnable()
	{
	}

	void CHierarchyWindow::OnInspectorGUI()
	{
		const Havtorn::SEditorLayout& layout = Manager->GetEditorLayout();
		
		const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + layout.HierarchyViewPosition.X, mainViewport->WorkPos.y + layout.HierarchyViewPosition.Y));
		ImGui::SetNextWindowSize(ImVec2(layout.HierarchyViewSize.X, layout.HierarchyViewSize.Y));
		
		if (ImGui::Begin(Name(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			Havtorn::I32 index = 0;
			auto& entities = Havtorn::GEngine::GetWorld()->GetEntities();
			
			for (auto& entity : entities) 
			{
				// TODO.AG: Temporary solution to ignore listing DebugShapes!
				if (entity->Name.Contains("hie_"))
				{
					continue;
				}
		
				ImGui::PushID(static_cast<int>(entity->ID));
				if (ImGui::Selectable(entity->Name.c_str(), index == SelectedIndex, ImGuiSelectableFlags_None)) 
				{
					SelectedIndex = index;
					Manager->SetSelectedEntity(entity);
				}
		
				ImGui::PopID();
			}
		}
		ImGui::End();
	}

	void CHierarchyWindow::OnDisable()
	{
	}
}