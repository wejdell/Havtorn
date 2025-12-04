// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

namespace Havtorn
{
	namespace HexRune
	{
		struct SScript;
	}

	constexpr F32 StartingZoom = 1.0f;

	class CMaterialTool : public CWindow
	{
	public:
		CMaterialTool(const char* displayName, CEditorManager* manager);
		~CMaterialTool() override = default;

		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

		void OpenMaterial(SEditorAssetRepresentation* asset);
		void CloseMaterial();

		void OnZoomInput(const SInputAxisPayload payload);
		void HandleAxisInput(const SInputAxisPayload payload);
		void ToggleFreeCam(const SInputActionPayload payload);

		void RenderMaterial();

	private:
		SEditorAssetRepresentation* CurrentMaterial = nullptr;
		SEngineGraphicsMaterial MaterialData;
		CRenderTexture* MaterialRender = nullptr;

		SAssetReference PreviewSkylightAssetRef = SAssetReference("Assets/Textures/Cubemaps/CubemapTheVisit.hva");
		STextureAsset* PreviewSkylight = nullptr;
		SColor PreviewLightColor = { 212.0f / 255.0f, 175.0f / 255.0f, 55.0f / 255.0f, 1.0f };
		F32 PreviewLightIntensity = 0.25f;

		U64 MaterialToolRenderID = 80090;
		U32 MaterialToolPreviewAssetID = 80100;
		SVector RotationInput = SVector::Zero;
		F32 CurrentZoom = StartingZoom;
		bool IsOrbiting = false;
		bool IsHoveringViewport = false;
		bool IsHoveringWindow = false;
	};
}
