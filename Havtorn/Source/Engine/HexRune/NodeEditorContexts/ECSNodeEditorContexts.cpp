// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ECSNodeEditorContexts.h"
#include "HexRune/HexRune.h"
#include "../ECSNodes/ECSNodes.h"

#include <GUI.h>

namespace Havtorn
{
	namespace HexRune
	{
		SEntityLoopNodeEditorContext SEntityLoopNodeEditorContext::Context = {};

		SEntityLoopNodeEditorContext::SEntityLoopNodeEditorContext()
		{
			Name = "For Each Loop (Entity)";
			Category = "General";
		}

		SNode* SEntityLoopNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SEntityLoopNode>(existingID, TypeID);
			script->AddEditorContext<SEntityLoopNodeEditorContext>(node->UID);
			return node;
		}

		SComponentLoopNodeEditorContext SComponentLoopNodeEditorContext::Context = {};

		SComponentLoopNodeEditorContext::SComponentLoopNodeEditorContext()
		{
			Name = "For Each Loop (Component)";
			Category = "General";
		}

		SNode* SComponentLoopNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SComponentLoopNode>(existingID, TypeID);
			script->AddEditorContext<SComponentLoopNodeEditorContext>(node->UID);
			return node;
		}

		SPrintEntityNameNodeEditorContext SPrintEntityNameNodeEditorContext::Context = {};
		SPrintEntityNameNodeEditorContext::SPrintEntityNameNodeEditorContext()
		{
			Name = "Print Entity Name";
			Category = "General";
			Color = SColor::Teal;
		}
		SNode* SPrintEntityNameNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SPrintEntityNameNode>(existingID, TypeID);
			script->AddEditorContext<SPrintEntityNameNodeEditorContext>(node->UID);
			return node;
		}

		SSetStaticMeshNodeEditorContext SSetStaticMeshNodeEditorContext::Context = {};
		SSetStaticMeshNodeEditorContext::SSetStaticMeshNodeEditorContext()
		{
			Name = "Set Static Mesh";
			Category = "ECS";
			Color = SColor::Orange;
		}
		SNode* SSetStaticMeshNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SSetStaticMeshNode>(existingID, TypeID);
			script->AddEditorContext<SSetStaticMeshNodeEditorContext>(node->UID);
			return node;
		}

		STogglePointLightNodeEditorContext STogglePointLightNodeEditorContext::Context = {};
		STogglePointLightNodeEditorContext::STogglePointLightNodeEditorContext()
		{
			Name = "Toggle Point Light";
			Category = "ECS";
			Color = SColor::Orange;
		}
		SNode* STogglePointLightNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<STogglePointLightNode>(existingID, TypeID);
			script->AddEditorContext<STogglePointLightNodeEditorContext>(node->UID);
			return node;
		}

		SOnBeginOverlapNodeEditorContext SOnBeginOverlapNodeEditorContext::Context = {};
		SOnBeginOverlapNodeEditorContext::SOnBeginOverlapNodeEditorContext()
		{
			Name = "On Begin Overlap";
			Category = "ECS";
			Color = SColor::Red;
		}
		SNode* SOnBeginOverlapNodeEditorContext::AddNode(SScript* script, const U64 /*existingID*/) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SOnBeginOverlapNode>(OnBeginOverlapNodeID, TypeID);
			script->AddEditorContext<SOnBeginOverlapNodeEditorContext>(node->UID);
			return node;
		}

		SOnEndOverlapNodeEditorContext SOnEndOverlapNodeEditorContext::Context = {};
		SOnEndOverlapNodeEditorContext::SOnEndOverlapNodeEditorContext()
		{
			Name = "On End Overlap";
			Category = "ECS";
			Color = SColor::Red;
		}
		SNode* SOnEndOverlapNodeEditorContext::AddNode(SScript* script, const U64 /*existingID*/) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SOnEndOverlapNode>(OnEndOverlapNodeID, TypeID);
			script->AddEditorContext<SOnEndOverlapNodeEditorContext>(node->UID);
			return node;
		}
	}
}
