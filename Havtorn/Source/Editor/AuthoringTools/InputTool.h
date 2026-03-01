// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

namespace Havtorn
{
	struct SEditorAssetRepresentation;

	enum class EInputDataType 
	{
		Button,
	};

	struct SInputToolOperation
	{
		EInputDataType InputDataType;
	};

	class CInputTool : public CWindow
	{
	public:
		CInputTool(const char* displayName, CEditorManager* manager);
		~CInputTool() override = default;

		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

		void OpenInputAsset(SEditorAssetRepresentation* asset);
	};
}
