// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

#include <GUI.h>

namespace Havtorn
{
	class CHierarchyWindow : public CWindow
	{
		struct SEditData
		{
			I64 QueuedRemovalIndex = -1;
			I64 DoubleClickIndex = -1;
		};

	public:
		CHierarchyWindow(const char* displayName, CEditorManager* manager);
		~CHierarchyWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:
		void FilterChildrenFromList(const CScene* scene, const std::vector<SEntity>& children, std::vector<SEntity>& filteredEntities);
		void InspectEntities(const CScene* scene, const std::vector<SEntity>& entities);

		void Header();
		void Body(std::vector<Ptr<CScene>>& scenes, SEditData& editData);
		void Footer(std::vector<Ptr<CScene>>& scenes, SEditData& editData);
		void SceneAssetDrag();

		void Edit(const SEditData& editData);

	private:
		SGuiTextFilter Filter = SGuiTextFilter();
	};
}
