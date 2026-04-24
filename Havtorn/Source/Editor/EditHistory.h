// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "EditActions/EditAction.h"

#include <GeneralUtilities.h>
#include <MetaCommand/MetaCommandRouter.h>

#include <GUI.h>

namespace Havtorn
{
	class CEditorManager;

	struct SEditActionTreeNode
	{
		Ref<SEditActionTreeNode> Parent = nullptr;
		std::vector<Ref<SEditActionTreeNode>> Children;
		Ref<SEditAction> Action;
	};

	class CEditHistory : public IMetaCommandParser
	{
	public:
		CEditHistory() = delete;
		CEditHistory(CEditorManager* manager);
		~CEditHistory();

		void RenderNode(const Ref<SEditActionTreeNode>& node, Ref<SEditActionTreeNode>& newNodeTarget, U16& guiID) const;
		void Render();
		virtual void Parse(const SMetaCommand& command) override;

		void Undo();
		void Redo(const U8 childIndex = 0);
		void ClearActions();

	private:
		void DownwardSearch(Ref<SEditActionTreeNode>& node, const Ref<SEditActionTreeNode>& searchTarget, bool& targetFound, std::vector<U8>& traversalPath);
		void GoToNode(Ref<SEditActionTreeNode>& node);

		Ref<SEditAction> MakeEditAction(const SMetaCommand& command) const;

		CEditorManager* Manager = nullptr;
		Ref<SEditActionTreeNode> HeadNode = nullptr;
		Ref<SEditActionTreeNode> TailNode = nullptr;

		SGuiTextFilter SearchFilter;

		const U32 EditorNavigationDomain = UGeneralUtils::HashString("EditorNavigation");
		const U32 BrowseFolderCommand = UGeneralUtils::HashString("BrowseFolder");

		const U32 SceneManipulationDomain = UGeneralUtils::HashString("SceneManipulation");
		const U32 ChangeScenesCommand = UGeneralUtils::HashString("ChangeScenes");

		const U32 EntityManipulationDomain = UGeneralUtils::HashString("EntityManipulation");
		const U32 ChangeEntityCommand = UGeneralUtils::HashString("ChangeEntity");
		const U32 ChangeComponentCommand = UGeneralUtils::HashString("ChangeComponent");
	};

	template<typename T, typename ...Params>
	SMetaCommand MakeEditActionCommand(Params ...params)
	{
		return SMetaCommand();
	}
}
