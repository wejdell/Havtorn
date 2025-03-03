// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "HierarchyWindow.h"
#include "EditorManager.h"

#include <Engine.h>
#include <Scene/Scene.h>
#include <ECS/Entity.h>
#include <MathTypes/EngineMath.h>
#include <CoreTypes.h>
#include <ECS/Components/MetaDataComponent.h>

#include <ECS/Systems/DebugShapeSystem.h>

namespace Havtorn
{
	CHierarchyWindow::CHierarchyWindow(const char* displayName, CEditorManager* manager)
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
		const SEditorLayout& layout = Manager->GetEditorLayout();
		
		const SGuiViewport* mainViewport = GUI::GetMainViewport();
		GUI::SetNextWindowPos(SVector2<F32>(mainViewport->WorkPos.x + layout.HierarchyViewPosition.X, mainViewport->WorkPos.y + layout.HierarchyViewPosition.Y));
		GUI::SetNextWindowSize(SVector2<F32>(layout.HierarchyViewSize.X, layout.HierarchyViewSize.Y));
		
		if (GUI::Begin(Name(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			Filter.Draw("Search");
			GUI::Separator();

			CScene* scene = Manager->GetCurrentScene();
			if (!scene)
			{
				GUI::End();
				return;
			}

			GUI::Text(scene->GetSceneName().c_str());

			I32 index = 0;
			const std::vector<SEntity>& entities = GEngine::GetWorld()->GetEntities();
			
			for (const SEntity& entity : entities)
			{
				if (!entity.IsValid())
					continue;
		
				const SMetaDataComponent* metaDataComp = scene->GetComponent<SMetaDataComponent>(entity);
				const std::string entryString = metaDataComp->IsValid() ? metaDataComp->Name.AsString() : "Selected";

				if (Filter.IsActive() && !Filter.PassFilter(entryString.c_str()))
					continue;

				GUI::PushID(STATIC_I32(entity.GUID));
				if (GUI::Selectable(entryString.c_str(), index == SelectedIndex, ImGuiSelectableFlags_None))
				{
					SelectedIndex = index;
					Manager->SetSelectedEntity(entity);
				}
				GUI::PopID();
			}
		}
		GUI::End();
	}

	void CHierarchyWindow::OnDisable()
	{
	}
}