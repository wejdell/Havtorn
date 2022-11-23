// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "HierarchyWindow.h"
#include "EditorManager.h"

#include "Scene/Scene.h"
#include "ECS/Entity.h"
#include "Core/MathTypes/EngineMath.h"
#include "Core/CoreTypes.h"

#include <ECS/Systems/DebugShapeSystem.h>

#include <imgui.h>

namespace ImGui
{
	CHierarchyWindow::CHierarchyWindow(const char* name, Havtorn::CScene* scene, Havtorn::CEditorManager* manager)
		: CWindow(name, manager)
		, Scene(scene)
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
			auto& entities = Scene->GetEntities();
			
			for(Havtorn::U64 i = Havtorn::UDebugShapeSystem::MaxShapes; i < entities.size(); i++)
			{
				auto& entity = entities[i];
				// TODO.AG: Temporary solution to ignore listing DebugShapes!
				//if (entity->Name.Contains("hie_"))
				//{
				//	continue;
				//}
		
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