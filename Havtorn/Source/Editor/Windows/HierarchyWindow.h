// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

#include <GUI.h>

namespace Havtorn
{
	enum class EEditorTexture;

	class CHierarchyWindow : public CWindow
	{
		struct SEditData
		{
			I64 QueuedRemovalIndex = -1;
			I64 HoveredIndex = -1;
		};

	public:
		CHierarchyWindow(const char* displayName, CEditorManager* manager);
		~CHierarchyWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:
		void FilterChildrenFromList(const CScene* scene, const std::vector<SEntity>& children, std::vector<SEntity>& filteredEntities);
		void InspectEntities(CScene* scene, const std::vector<SEntity>& entities);

		void Header();
		void Body(std::vector<Ptr<CScene>>& scenes, SEditData& editData);
		void Footer(std::vector<Ptr<CScene>>& scenes, SEditData& editData);
		void SceneAssetDrag();

		void Edit(SEditData& editData);

		std::vector<EEditorTexture> GetRelevantComponentIcons(const CScene* scene, const SEntity& entity);
	private:
		SGuiTextFilter Filter = SGuiTextFilter();
		I64 SelectedIndex = -1;
		
		const SVector2<F32> ComponentIconSize = SVector2<F32>(12.0f, 14.0f);
		const U64 MaxComponentIconsToAdd = 3llu;
		const F32 ComponentIconCursorOffsetX = 20.0f;
		std::string PerComponentIconTextOffset = "";
	};
}
