// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"
#include <Assets/AssetReference.h>
#include <Assets/RuntimeAssets/InputAsset.h>

namespace Havtorn
{
	struct SEditorAssetRepresentation;

	struct SAxis;

	class CInputTool : public CWindow
	{
	public:
		CInputTool(const char* displayName, CEditorManager* manager);
		~CInputTool() override = default;

		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

		void OpenInputAsset(SEditorAssetRepresentation* asset);

	private:
		void DrawInputTable();
		void DrawAxisGUI(const char* label, SAxis& axisValue, EInputActivationType& mappingActivationType);
		// Returns true if button press assignment is ongoing
		bool DrawAssignButtonKeyElement(const char* label, EInputButton* key);
		
		std::string AssetName;
		SAssetReference AssetReference;
		SInputAsset* InputAsset = nullptr;

		EInputButton* CurrentButtonBeingAssigned = nullptr;

		const U64 InputToolID = 107001; //1Input7Tool
	};
}
