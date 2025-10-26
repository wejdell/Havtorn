// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

#include <GUI.h>

namespace Havtorn
{
	class CHierarchyWindow : public CWindow
	{
	public:
		CHierarchyWindow(const char* displayName, CEditorManager* manager);
		~CHierarchyWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

		void FilterChildrenFromList(const CScene* scene, const std::vector<SEntity>& children, std::vector<SEntity>& filteredEntities);
		void InspectEntities(const CScene* scene, const std::vector<SEntity>& entities);

		void SceneAssetDrag();

	private:
		SGuiTextFilter Filter = SGuiTextFilter();
	};
}
