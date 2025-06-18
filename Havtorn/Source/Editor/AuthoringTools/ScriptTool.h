// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

namespace Havtorn
{
	namespace HexRune
	{
		struct SScript;
	
		struct SComponentsNode : public SNode
		{
			virtual void Construct() override;
			virtual I8 OnExecute() override;
		};

		struct STestNode : public SNode
		{
			virtual void Construct() override;
			virtual I8 OnExecute() override;
			virtual bool IsStartNode() const override { return true; }
		};

		struct SLiteralFloatNode : public SNode
		{
			virtual void Construct() override;
			virtual I8 OnExecute() override;
		};

		struct SLiteralStringNode : public SNode
		{
			virtual void Construct() override;
			virtual I8 OnExecute() override;
		};
	}

	class CScriptTool : public CWindow
	{
	public:
		CScriptTool(const char* displayName, CEditorManager* manager);
		~CScriptTool() override = default;

		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

		void OpenScript(HexRune::SScript* script);
		void CloseScript();

	private:
		void LoadGUIElements();
		void CommitEdit(const SNodeOperation& edit);

		HexRune::SScript* CurrentScript = nullptr;
		std::vector<SGUINode> GUINodes;
		std::vector<SGUILink> GUILinks;
		std::vector<SGUINodeContext> GUIContexts;
		SGuiTextFilter Filter;
	};
}
