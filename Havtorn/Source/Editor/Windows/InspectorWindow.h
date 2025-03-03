// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

#include <MathTypes/Vector.h>

namespace Havtorn
{
	struct SComponentViewResult;
	class CScene;

	template<typename T = F32>
	struct SVector2;

	class CInspectorWindow : public CWindow
	{
	public:
		CInspectorWindow(const char* displayName, CEditorManager* manager);
		~CInspectorWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:
		void UpdateTransformGizmo(const SComponentViewResult& result);
		void ViewManipulation(SMatrix& outCameraView, const SVector2<F32>& windowPosition, const SVector2<F32>& windowSize);
		void InspectAssetComponent(const SComponentViewResult& result);
		void IterateAssetRepresentations(const SComponentViewResult& result, const std::vector<std::string>& assetNames, const std::vector<std::string>& assetLabels, const std::string& modalNameToOpen);
		void OpenSelectMeshAssetModal(const SComponentViewResult& result);
		void OpenSelectTextureAssetModal(const SComponentViewResult& result);
		void OpenSelectMaterialAssetModal(const SComponentViewResult& result);
		void OpenAssetTool(const SComponentViewResult& result);
		void HandleTextureAssetModal(const std::string& pathToSearch, U16& textureReference);
		
		// TODO.NR: Support adding and removing components through the editor. Unsolved problem.
		void OpenAddComponentModal();

	private:
		const std::string SelectMeshAssetModalName = "Select Mesh Asset";
		const std::string SelectMaterialAssetModalName = "Select Material Asset";
		const std::string SelectTextureAssetModalName = "Select Texture Asset";

		CScene* Scene = nullptr;
		SEntity SelectedEntity = SEntity::Null;
		U8 AssetPickedIndex = 0;
	};
}
