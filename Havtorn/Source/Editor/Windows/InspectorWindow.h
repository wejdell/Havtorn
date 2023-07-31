// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"
#include <map>
#include <functional>

namespace Havtorn
{
	enum class EComponentType;
	class CScene;
	struct SStaticMeshComponent;
	struct SMaterialComponent;
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
		void TryInspectComponent(const Havtorn::SEntity* selectedEntity, Havtorn::EComponentType componentType);

		void InspectTransformComponent();
		void InspectStaticMeshComponent();
		void InspectCameraComponent();
		void InspectCameraControllerComponent();
		void InspectMaterialComponent();
		void InspectEnvironmentLightComponent();
		void InspectDirectionalLightComponent();
		void InspectPointLightComponent();
		void InspectSpotLightComponent();
		void InspectVolumetricLightComponent();
		void InspectDecalComponent();
		void InspectSpriteComponent();
		void InspectTransform2DComponent();

		void OpenSelectMeshAssetModal(Havtorn::I64 staticMeshComponentIndex);
		void OpenSelectTextureAssetModal(Havtorn::U16& textureRefToChange);
		void OpenSelectMaterialAssetModal(Havtorn::SMaterialComponent* materialComponentToChange, Havtorn::U8 materialIndex);
		void OpenAddComponentModal();
		void RemoveComponentButton(Havtorn::EComponentType componentType);

	private:
		std::map<Havtorn::EComponentType, std::function<void()>> InspectionFunctions;
		Havtorn::CScene* Scene = nullptr;
		Havtorn::SVector2<Havtorn::F32> TexturePreviewSize = { 64.0f, 64.0f };
		Havtorn::SVector2<Havtorn::F32> DummySize = { 0.0f, 0.5f };
		Havtorn::U64 SelectedEntityIndex = 0;
		Havtorn::F32 SlideSpeed = 0.1f;
		Havtorn::U16 MaterialRefToChangeIndex = 0;
		Havtorn::U8 MaterialToChangeIndex = 0;
		bool SelectTextureWindowOpen = false;
	};
}
