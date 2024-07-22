// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"
//#include <map>
//#include <functional>

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
		void UpdateCameraGizmo(const Havtorn::SComponentViewResult& result);
		void OpenSelectMeshAssetModal(const Havtorn::SComponentViewResult& result);
		void OpenSelectTextureAssetModal(const Havtorn::SComponentViewResult& result);
		void OpenSelectMaterialAssetModal(const Havtorn::SComponentViewResult& result);
		void OpenAssetTool(const Havtorn::SComponentViewResult& result);
		void HandleTextureAssetModal(const std::string& pathToSearch, Havtorn::U16& textureReference);
		void OpenAddComponentModal();
		void RemoveComponentButton(/*Havtorn::EComponentType componentType*/);

	private:
		Havtorn::CScene* Scene = nullptr;
		Havtorn::SVector2<Havtorn::F32> TexturePreviewSize = { 64.0f, 64.0f };
		Havtorn::SVector2<Havtorn::F32> DummySize = { 0.0f, 0.5f };
		Havtorn::SEntity SelectedEntity = Havtorn::SEntity::Null;
		Havtorn::F32 SlideSpeed = 0.1f;
		Havtorn::U16 MaterialRefToChangeIndex = 0;
		Havtorn::U8 MaterialToChangeIndex = 0;
		bool SelectTextureWindowOpen = false;
	};
}
