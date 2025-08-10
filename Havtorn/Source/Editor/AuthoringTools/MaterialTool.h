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

	private:
		SEditorAssetRepresentation* CurrentMaterial = nullptr;
		SEngineGraphicsMaterial MaterialData;
		CRenderTexture MaterialRender;

		SVector RotationInput = SVector::Zero;
		F32 CurrentZoom = StartingZoom;
		bool IsOrbiting = false;
	};
}
