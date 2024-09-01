// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

namespace Havtorn
{
	struct SComponentViewResult;
	class CScene;
}

namespace ImGui
{
	class CInspectorWindow : public CWindow
	{
	public:
		CInspectorWindow(const char* displayName, Havtorn::CEditorManager* manager);
		~CInspectorWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:
		void UpdateTransformGizmo(const Havtorn::SComponentViewResult& result);
		void InspectAssetComponent(const Havtorn::SComponentViewResult& result);
		void IterateAssetRepresentations(const Havtorn::SComponentViewResult& result, const std::vector<std::string>& assetNames, const std::vector<std::string>& assetLabels, const std::string& modalNameToOpen);
		void OpenSelectMeshAssetModal(const Havtorn::SComponentViewResult& result);
		void OpenSelectTextureAssetModal(const Havtorn::SComponentViewResult& result);
		void OpenSelectMaterialAssetModal(const Havtorn::SComponentViewResult& result);
		void OpenAssetTool(const Havtorn::SComponentViewResult& result);
		void HandleTextureAssetModal(const std::string& pathToSearch, Havtorn::U16& textureReference);
		
		// TODO.NR: Support adding and removing components through the editor. Unsolved problem.
		void OpenAddComponentModal();
		void RemoveComponentButton(/*Havtorn::EComponentType componentType*/);

	private:
		const std::string SelectMeshAssetModalName = "Select Mesh Asset";
		const std::string SelectMaterialAssetModalName = "Select Material Asset";
		const std::string SelectTextureAssetModalName = "Select Texture Asset";

		Havtorn::CScene* Scene = nullptr;
		Havtorn::SEntity SelectedEntity = Havtorn::SEntity::Null;
		Havtorn::U8 AssetPickedIndex = 0;
	};
}
