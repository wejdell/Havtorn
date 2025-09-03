// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

#include <MathTypes/Vector.h>

namespace Havtorn
{
	struct SComponentViewResult;
	struct SEditorAssetRepresentation;
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
		void InspectAssetComponent(SComponentViewResult& result);
		void OpenAssetTool(const SComponentViewResult& result);
		
		void OpenAddComponentModal(const SEntity& entity);

		void UpdateAssetContextMenu();

	private:
		CScene* Scene = nullptr;
		U8 AssetPickedIndex = 0;
		SMatrix DeltaMatrix = SMatrix::Identity;
		SAssetReference* ContextMenuAssetRef = nullptr;
		U64 ContextMenuAssetRequester = 0;
		bool IsContextMenuRefHovered = false;
	};
}
