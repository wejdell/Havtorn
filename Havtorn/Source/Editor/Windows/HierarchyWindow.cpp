// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "HierarchyWindow.h"
#include "EditorManager.h"

#include "Engine.h"
#include "Scene/Scene.h"
#include "ECS/Entity.h"
#include "Core/MathTypes/EngineMath.h"
#include "Core/CoreTypes.h"
#include "ECS/Components/MetaDataComponent.h"

#include <ECS/Systems/DebugShapeSystem.h>

#include <Core/imgui.h>

namespace ImGui
{
	CHierarchyWindow::CHierarchyWindow(const char* displayName, Havtorn::CEditorManager* manager)
		: CWindow(displayName, manager)
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
			Filter.Draw("Search");
			ImGui::Separator();

			Havtorn::CScene* scene = Manager->GetCurrentScene();
			if (!scene)
			{
				ImGui::End();
				return;
			}

			ImGui::Text(scene->GetSceneName().c_str());

			Havtorn::I32 index = 0;
			const std::vector<Havtorn::SEntity>& entities = Havtorn::GEngine::GetWorld()->GetEntities();
			
			for (const Havtorn::SEntity& entity : entities)
			{
				if (!entity.IsValid())
					continue;
		
				const Havtorn::SMetaDataComponent* metaDataComp = scene->GetComponent<Havtorn::SMetaDataComponent>(entity);
				const std::string entryString = metaDataComp->IsValid() ? metaDataComp->Name.AsString() : "Selected";

				if (Filter.IsActive() && !Filter.PassFilter(entryString.c_str()))
					continue;

				ImGui::PushID(static_cast<Havtorn::I32>(entity.GUID));
				if (ImGui::Selectable(entryString.c_str(), index == SelectedIndex, ImGuiSelectableFlags_None))
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