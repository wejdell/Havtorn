// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"
#include <Assets/AssetReference.h>
#include <Assets/RuntimeAssets/InputAsset.h>

namespace Havtorn
{
	struct SEditorAssetRepresentation;

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
		
		std::string AssetName;
		SAssetReference AssetReference;
		SInputAsset* InputAsset = nullptr;

		const U64 InputToolID = 107001; //1Input7Tool
	};
}
