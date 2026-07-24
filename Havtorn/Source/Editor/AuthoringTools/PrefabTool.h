// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

namespace Havtorn
{
	class CPrefabTool : public CWindow
	{
	public:
		CPrefabTool(const char* displayName, CEditorManager* manager);
		~CPrefabTool() override = default;

		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDeferredExit() override;
		void OnDisable() override;

		void OpenPrefab(SEditorAssetRepresentation* asset);
		void ClosePrefab();

		void RenderPreviewSettings();
		void UpdateViewMatrix();
		void RenderPrefab();

		void HandleAxisInput(const SInputAxisPayload payload);
		void ToggleFreeCam(const SInputActionPayload payload);
	
		CScene* GetWorkingScene() const;
		const SMatrix& GetViewMatrix() const;
		const SMatrix& GetProjectionMatrix() const;
		const SVector2<F32>& GetPreviewWindowPosition() const;
		const SVector2<F32>& GetPreviewWindowDimensions() const;

	private:
		SAssetReference CurrentPrefabAssetRef = SAssetReference();
		SPrefabAsset* PrefabData = nullptr;
		CRenderTexture* PrefabRender = nullptr;

		SGuiTextFilter Filter = SGuiTextFilter();
		I64 SelectedIndex = -1;

		const SVector2<F32> ComponentIconSize = SVector2<F32>(12.0f, 14.0f);
		const U64 MaxComponentIconsToAdd = 3llu;
		const F32 ComponentIconCursorOffsetX = 20.0f;
		std::string PerComponentIconTextOffset = "";
		
		SAssetReference PreviewSkylightAssetRef = SAssetReference("Resources/DefaultSkybox.hva");
		STextureCubeAsset* PreviewSkylight = nullptr;

		SVector PreviewLightDirection = { 1.0f, 0.0f, -1.0f };
		SColor PreviewLightColor = { 212.0f / 255.0f, 175.0f / 255.0f, 55.0f / 255.0f, 1.0f };
		F32 PreviewLightIntensity = 0.25f;

		static constexpr F32 MaxPitchDegrees = 90.0f;

		SMatrix PreviewCameraViewMatrix = SMatrix::Identity;
		SMatrix PreviewCameraProjectionMatrix = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), 1.0f, 0.01f, 1000.0f);
		SVector2<F32> WindowPos = SVector2<F32>::Zero;
		SVector2<F32> PreviewWindowPosition = SVector2<F32>::Zero;
		SVector2<F32> PreviewWindowDimensions = SVector2<F32>::Zero;
		F32 PreviewCameraMaxMoveSpeed = 3.0f;
		F32 PreviewCameraRotationSpeed = 0.2f;
		F32 PreviewCameraAccelerationDuration = 0.2f;

		SVector PreviewCameraAccelerationDirection = SVector::Zero;
		F32 PreviewCameraCurrentPitch = 0.0f;
		F32 PreviewCameraCurrentYaw = 0.0f;
		F32 PreviewCameraCurrentAccelerationFactor = 0.0f;
		
		SVector PreviewCameraMoveInput = SVector::Zero;
		SVector PreviewCameraRotationInput = SVector::Zero;
		F32 PreviewCameraSpeedInput = 0.0f;

		U64 PrefabToolRenderID = 90100;
		U32 PrefabToolPreviewAssetID = 90200;

		bool IsFreeCamActive = false;
		bool IsHoveringViewport = false;
		bool IsHoveringWindow = false;
	};
}
