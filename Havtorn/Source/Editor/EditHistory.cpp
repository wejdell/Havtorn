// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "EditHistory.h"
#include "EditorManager.h"

#include "EditActions/BrowseFolderEditAction.h"
#include "EditActions/MoveTransformEditAction.h"

namespace Havtorn
{
    CEditHistory::CEditHistory(CEditorManager* manager)
        : Manager(manager)
    {
        ParserID = UGeneralUtils::HashString("EditHistory");
        UMetaCommandRouter::AddParser(this);

        HeadNode = std::make_shared<SEditActionTreeNode>(SEditActionTreeNode{ .Action = MakeEditAction(SMetaCommand("None")) });
        TailNode = HeadNode;
    }

    CEditHistory::~CEditHistory()
    {
        UMetaCommandRouter::RemoveParser(this);
    }

    void CEditHistory::RenderNode(const Ref<SEditActionTreeNode>& node, Ref<SEditActionTreeNode>& newNodeTarget, U16& guiID) const
    {
        const std::string commandDisplayName = node->Action->GetCompactName();

        if (SearchFilter.IsActive() && !SearchFilter.PassFilter(commandDisplayName.c_str()))
        {
            for (Ref<SEditActionTreeNode>& child : node->Children)
            {
                RenderNode(child, newNodeTarget, guiID);
            }
            return;
        }

        std::vector<ETreeNodeFlag> flags = { ETreeNodeFlag::SpanAvailWidth, ETreeNodeFlag::DefaultOpen, ETreeNodeFlag::OpenOnArrow, ETreeNodeFlag::DrawLinesToNodes };

        if (node->Children.empty())
            flags.emplace_back(ETreeNodeFlag::Leaf);

        if (node == TailNode)
            flags.emplace_back(ETreeNodeFlag::Selected);

        GUI::PushID(guiID++);
        if (GUI::TreeNodeEx(commandDisplayName.c_str(), flags))
        {
            if (GUI::IsItemClicked() && GUI::IsDoubleClick())
            {
                newNodeTarget = node;
            }

            for (Ref<SEditActionTreeNode>& child : node->Children)
            {
                RenderNode(child, newNodeTarget, guiID);
            }
            GUI::TreePop();
        }
        GUI::PopID();
    };

    void CEditHistory::Render()
    {
        SearchFilter.Draw("Search", 0); // TODO.NW: Figure out a nicer way of setting the width

        GUI::SameLine();
        if (GUI::Button("Clear"))
        {
            ClearActions();
        }

        GUI::Separator();

        if (GUI::BeginChild("Graph", SVector2<F32>(0.0f, 0.0f), { EChildFlag::AutoResizeY, EChildFlag::AlwaysAutoResize }))
        {
            U16 id = 0;
            Ref<SEditActionTreeNode> newNodeTarget = HeadNode;
            for (Ref<SEditActionTreeNode>& child : HeadNode->Children)
            {
                RenderNode(child, newNodeTarget, id);
            }

            if (newNodeTarget != HeadNode)
                GoToNode(newNodeTarget);
        }
        GUI::EndChild();
    }

    void CEditHistory::Parse(const SMetaCommand& command)
    {
        TailNode = TailNode->Children.emplace_back(std::make_shared<SEditActionTreeNode>(SEditActionTreeNode{ .Parent = TailNode, .Action = MakeEditAction(command) }));
    }

    void CEditHistory::Undo()
    {
        if (TailNode->Parent.get() == nullptr)
            return;

        TailNode->Action->ResolveAction(Manager, true);
        TailNode = TailNode->Parent;
    }

    void CEditHistory::Redo(const U8 childIndex)
    {
        if (TailNode->Children.empty())
            return;

        TailNode = TailNode->Children[childIndex];
        TailNode->Action->ResolveAction(Manager, false);
    }

    void CEditHistory::ClearActions()
    {
        HeadNode->Children.clear();
        TailNode = HeadNode;
    }

    void CEditHistory::DownwardSearch(Ref<SEditActionTreeNode>& node, const Ref<SEditActionTreeNode>& searchTarget, bool& targetFound, std::vector<U8>& traversalPath)
    {
        if (!targetFound)
            targetFound = node == searchTarget;

        if (targetFound)
        {
            if (targetFound && node->Parent != nullptr)
                traversalPath.push_back(STATIC_U8(std::distance(node->Parent->Children.begin(), std::ranges::find(node->Parent->Children, node))));
            
            return;
        }

        for (Ref<SEditActionTreeNode>& child : node->Children)
        {
            if (!targetFound)
                DownwardSearch(child, searchTarget, targetFound, traversalPath);
        }

        if (targetFound && node->Parent != nullptr)
            traversalPath.push_back(STATIC_U8(std::distance(node->Parent->Children.begin(), std::ranges::find(node->Parent->Children, node))));
    }

    void CEditHistory::GoToNode(Ref<SEditActionTreeNode>& node)
    {
        std::vector<U8> traversalPath;
        while (TailNode != node)
        {
            bool isTargetChild = false;
            DownwardSearch(TailNode, node, isTargetChild, traversalPath);

            if (isTargetChild)
            {
                Undo();
                std::ranges::reverse(traversalPath);
                for (const U8 childIndex : traversalPath)
                    Redo(childIndex);
            }
            else
            {
                Undo();
            }
        }
    }

    Ref<SEditAction> CEditHistory::MakeEditAction(const SMetaCommand& command) const
    {
        if (command.Domain == EditorNavigationDomain)
        {
            if (command.Command == BrowseFolderCommand)
            {
                if (command.Parameters.contains("From") && command.Parameters.contains("To"))
                    return std::make_shared<SBrowseFolderEditAction>(command);
            }
        }
        else if (command.Domain == EntityManipulationDomain)
        {
            if (command.Command == ChangeComponentCommand)
            {
                if (command.Parameters.contains("TransformStart"))
                    return std::make_shared<SMoveTransformEditAction>(command);
            }
        }

        return Ref<SEditAction>();
    }
}
