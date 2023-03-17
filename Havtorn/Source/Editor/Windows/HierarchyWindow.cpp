// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "HierarchyWindow.h"
//#include <imgui.h>
#include "EditorManager.h"

#include "Engine.h"
#include "Scene/Scene.h"
#include "ECS/Entity.h"
#include "Core/MathTypes/EngineMath.h"
#include "Core/CoreTypes.h"
#include "ECS/Components/MetaDataComponent.h"

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
			Havtorn::CScene* scene = Manager->GetCurrentScene();
			if (!scene)
			{
				ImGui::End();
				return;
			}

			Havtorn::I32 index = 0;
			auto& entities = Havtorn::GEngine::GetWorld()->GetEntities();
			
			for (auto& entity : entities) 
			{
				if (!entity.IsValid())
					continue;

				//// TODO.AG: Temporary solution to ignore listing DebugShapes!
				//if (entity->Name.Contains("hie_"))
				//{
				//	continue;
				//}
		
				const Havtorn::SMetaDataComponent& metaDataComp = scene->GetMetaDataComponents()[scene->GetSceneIndex(entity)];
				const std::string entryString = metaDataComp.IsInUse ? metaDataComp.Name.AsString() : "Selected";

				ImGui::PushID(static_cast<Havtorn::I32>(entity.GUID));
				if (ImGui::Selectable(entryString.c_str(), index == SelectedIndex, ImGuiSelectableFlags_None))
				{
					SelectedIndex = index;
					Manager->SetSelectedEntity(&entity);
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