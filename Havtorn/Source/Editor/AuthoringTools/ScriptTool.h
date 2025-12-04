// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

namespace Havtorn
{
	namespace HexRune
	{
		struct SScript;
	}

	class CScriptTool : public CWindow
	{
	public:
		CScriptTool(const char* displayName, CEditorManager* manager);
		~CScriptTool() override = default;

		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

		void OpenScript(SEditorAssetRepresentation* asset);
		void SaveScript();
		void CloseScript();

	private:
		void LoadGUIElements();
		void CommitEdit(const SNodeOperation& edit);

		void RenderScript();
		void RenderNodes();
		void HandleCreateAction();
		void HandleDeleteAction();
		void ContextMenu();
		void CommandQueue();

		SVector2<F32> GetNodeSize(const SGUINode& node);
		bool IsPinLinked(U64 id, const std::vector<SGUILink>& links);
		bool IsPinTypeLiteral(SGUIPin& pin);
		bool DrawLiteralTypePin(SGUIPin& pin);
		void DrawPinIcon(const SGUIPin& pin, bool connected, U8 alpha, bool highlighted);
		SColor GetPinTypeColor(EGUIPinType type);
		SGUINode* GetNodeFromPinID(U64 id, std::vector<SGUINode>& nodes);
		SGUIPin* GetPinFromID(U64 id, SGUINode& node);
		SGUIPin* GetPinFromID(U64 id, std::vector<SGUINode>& nodes);
		SGUIPin* GetOutputPinFromID(U64 id, std::vector<SGUINode>& nodes);

		SEditorAssetRepresentation* CurrentScriptRepresentation = nullptr;
		HexRune::SScript* CurrentScript = nullptr;

		std::vector<SGUINode> GUINodes;
		std::vector<SGUILink> GUILinks;
		std::vector<SGUINodeContext> GUIContexts;

		EGUIPinType CurrentDragPinType = EGUIPinType::Unknown;
		SGuiTextFilter Filter;
		SNodeOperation Edit;
		SGUIDataBinding DataBindingCandidate;

		const F32 HeaderHeight = 12.0f;
		const F32 PinNameOffset = 4.0f;

		bool IsHoveringWindow = false;
	};
}
