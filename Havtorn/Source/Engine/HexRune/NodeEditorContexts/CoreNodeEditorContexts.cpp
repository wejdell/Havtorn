// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CoreNodeEditorContexts.h"
#include "HexRune/HexRune.h"
#include "../CoreNodes/CoreNodes.h"

#include <GUI.h>

namespace Havtorn
{
	namespace HexRune
	{
		SDataBindingNodeEditorContext::SDataBindingNodeEditorContext(SScript* script, const U64 dataBindingID)
			: DataBindingID(dataBindingID)
		{
			auto it = &(*std::ranges::find_if(script->DataBindings, [dataBindingID](SScriptDataBinding& binding) { return binding.UID == dataBindingID; }));
			Name = it->Name;
			Category = "Data Bindings";
			Color = SColor::Orange;
		}

		SNode* SDataBindingNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SDataBindingNode>(existingID, DataBindingID);
			script->AddEditorContext<SDataBindingNodeEditorContext>(node->UID, script, DataBindingID);
			return node;
		}

		SBranchNodeEditorContext SBranchNodeEditorContext::Context = {};

		SBranchNodeEditorContext::SBranchNodeEditorContext()
		{
			Name = "Branch";
			Category = "General";
		}

		SNode* SBranchNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SBranchNode>(existingID);
			script->AddEditorContext<SBranchNodeEditorContext>(node->UID);
			return node;
		}

		SSequenceNodeEditorContext SSequenceNodeEditorContext::Context = {};

		SSequenceNodeEditorContext::SSequenceNodeEditorContext()
		{
			Name = "Sequence";
			Category = "General";
		}

		SNode* SSequenceNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SSequenceNode>(existingID);
			script->AddEditorContext<SSequenceNodeEditorContext>(node->UID);
			return node;
		}

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

			SNode* node = script->AddNode<SEntityLoopNode>(existingID);
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

			SNode* node = script->AddNode<SComponentLoopNode>(existingID);
			script->AddEditorContext<SComponentLoopNodeEditorContext>(node->UID);
			return node;
		}

		SDelayNodeEditorContext SDelayNodeEditorContext::Context = {};

		SDelayNodeEditorContext::SDelayNodeEditorContext()
		{
			Name = "Delay";
			Category = "General";
			Color = SColor::Teal;
		}

		SNode* SDelayNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SDelayNode>(existingID);
			script->AddEditorContext<SDelayNodeEditorContext>(node->UID);
			return node;
		}

		SBeginPlayNodeEditorContext SBeginPlayNodeEditorContext::Context = {};

		SBeginPlayNodeEditorContext::SBeginPlayNodeEditorContext()
		{
			Name = "Begin Play";
			Category = "General";
			Color = SColor::Red;
		}

		SNode* SBeginPlayNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SBeginPlayNode>(existingID);
			script->AddEditorContext<SBeginPlayNodeEditorContext>(node->UID);
			return node;
		}

		STickNodeEditorContext STickNodeEditorContext::Context = {};

		STickNodeEditorContext::STickNodeEditorContext()
		{
			Name = "Tick";
			Category = "General";
			Color = SColor::Red;
		}

		SNode* STickNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<STickNode>(existingID);
			script->AddEditorContext<STickNodeEditorContext>(node->UID);
			return node;
		}

		SEndPlayNodeEditorContext SEndPlayNodeEditorContext::Context = {};

		SEndPlayNodeEditorContext::SEndPlayNodeEditorContext()
		{
			Name = "End Play";
			Category = "General";
			Color = SColor::Red;
		}

		SNode* SEndPlayNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SEndPlayNode>(existingID);
			script->AddEditorContext<SEndPlayNodeEditorContext>(node->UID);
			return node;
		}

		SPrintStringNodeEditorContext SPrintStringNodeEditorContext::Context = {};

		SPrintStringNodeEditorContext::SPrintStringNodeEditorContext()
		{
			Name = "Print String";
			Category = "General";
			Color = SColor::Teal;
		}

		SNode* SPrintStringNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SPrintStringNode>(existingID);
			script->AddEditorContext<SPrintStringNodeEditorContext>(node->UID);
			return node;
		}

		SAppendStringNodeEditorContext SAppendStringNodeEditorContext::Context = {};

		SAppendStringNodeEditorContext::SAppendStringNodeEditorContext()
		{
			Name = "Append String";
			Category = "General";
			Color = SColor::Green;
		}

		SNode* SAppendStringNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SAppendStringNode>(existingID);
			script->AddEditorContext<SAppendStringNodeEditorContext>(node->UID);
			return node;
		}

		SFloatLessThanNodeEditorContext SFloatLessThanNodeEditorContext::Context = {};

		SFloatLessThanNodeEditorContext::SFloatLessThanNodeEditorContext()
		{
			Name = "< (Float)";
			Category = "Math";
		}

		SNode* SFloatLessThanNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SFloatLessThanNode>(existingID);
			script->AddEditorContext<SFloatLessThanNodeEditorContext>(node->UID);
			return node;
		}

		SFloatMoreThanNodeEditorContext SFloatMoreThanNodeEditorContext::Context = {};

		SFloatMoreThanNodeEditorContext::SFloatMoreThanNodeEditorContext()
		{
			Name = "> (Float)";
			Category = "Math";
		}

		SNode* SFloatMoreThanNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SFloatMoreThanNode>(existingID);
			script->AddEditorContext<SFloatMoreThanNodeEditorContext>(node->UID);
			return node;
		}

		SFloatLessOrEqualNodeEditorContext SFloatLessOrEqualNodeEditorContext::Context = {};

		SFloatLessOrEqualNodeEditorContext::SFloatLessOrEqualNodeEditorContext()
		{
			Name = "<= (Float)";
			Category = "Math";
		}

		SNode* SFloatLessOrEqualNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SFloatLessOrEqualNode>(existingID);
			script->AddEditorContext<SFloatLessOrEqualNodeEditorContext>(node->UID);
			return node;
		}

		SFloatMoreOrEqualNodeEditorContext SFloatMoreOrEqualNodeEditorContext::Context = {};

		SFloatMoreOrEqualNodeEditorContext::SFloatMoreOrEqualNodeEditorContext()
		{
			Name = ">= (Float)";
			Category = "Math";
		}

		SNode* SFloatMoreOrEqualNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SFloatMoreOrEqualNode>(existingID);
			script->AddEditorContext<SFloatMoreOrEqualNodeEditorContext>(node->UID);
			return node;
		}

		SFloatEqualNodeEditorContext SFloatEqualNodeEditorContext::Context = {};

		SFloatEqualNodeEditorContext::SFloatEqualNodeEditorContext()
		{
			Name = "== (Float)";
			Category = "Math";
		}

		SNode* SFloatEqualNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SFloatEqualNode>(existingID);
			script->AddEditorContext<SFloatEqualNodeEditorContext>(node->UID);
			return node;
		}

		SFloatNotEqualNodeEditorContext SFloatNotEqualNodeEditorContext::Context = {};

		SFloatNotEqualNodeEditorContext::SFloatNotEqualNodeEditorContext()
		{
			Name = "!= (Float)";
			Category = "Math";
		}

		SNode* SFloatNotEqualNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SFloatNotEqualNode>(existingID);
			script->AddEditorContext<SFloatNotEqualNodeEditorContext>(node->UID);
			return node;
		}

		SIntLessThanNodeEditorContext SIntLessThanNodeEditorContext::Context = {};

		SIntLessThanNodeEditorContext::SIntLessThanNodeEditorContext()
		{
			Name = "< (Int)";
			Category = "Math";
		}

		SNode* SIntLessThanNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SIntLessThanNode>(existingID);
			script->AddEditorContext<SIntLessThanNodeEditorContext>(node->UID);
			return node;
		}

		SIntMoreThanNodeEditorContext SIntMoreThanNodeEditorContext::Context = {};

		SIntMoreThanNodeEditorContext::SIntMoreThanNodeEditorContext()
		{
			Name = "> (Int)";
			Category = "Math";
		}

		SNode* SIntMoreThanNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SIntMoreThanNode>(existingID);
			script->AddEditorContext<SIntMoreThanNodeEditorContext>(node->UID);
			return node;
		}

		SIntLessOrEqualNodeEditorContext SIntLessOrEqualNodeEditorContext::Context = {};

		SIntLessOrEqualNodeEditorContext::SIntLessOrEqualNodeEditorContext()
		{
			Name = "<= (Int)";
			Category = "Math";
		}

		SNode* SIntLessOrEqualNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SIntLessOrEqualNode>(existingID);
			script->AddEditorContext<SIntLessOrEqualNodeEditorContext>(node->UID);
			return node;
		}

		SIntMoreOrEqualNodeEditorContext SIntMoreOrEqualNodeEditorContext::Context = {};

		SIntMoreOrEqualNodeEditorContext::SIntMoreOrEqualNodeEditorContext()
		{
			Name = ">= (Int)";
			Category = "Math";
		}

		SNode* SIntMoreOrEqualNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SIntMoreOrEqualNode>(existingID);
			script->AddEditorContext<SIntMoreOrEqualNodeEditorContext>(node->UID);
			return node;
		}

		SIntEqualNodeEditorContext SIntEqualNodeEditorContext::Context = {};

		SIntEqualNodeEditorContext::SIntEqualNodeEditorContext()
		{
			Name = "== (Int)";
			Category = "Math";
		}

		SNode* SIntEqualNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SIntEqualNode>(existingID);
			script->AddEditorContext<SIntEqualNodeEditorContext>(node->UID);
			return node;
		}

		SIntNotEqualNodeEditorContext SIntNotEqualNodeEditorContext::Context = {};

		SIntNotEqualNodeEditorContext::SIntNotEqualNodeEditorContext()
		{
			Name = "!= (Int)";
			Category = "Math";
		}

		SNode* SIntNotEqualNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SIntNotEqualNode>(existingID);
			script->AddEditorContext<SIntNotEqualNodeEditorContext>(node->UID);
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

			SNode* node = script->AddNode<SPrintEntityNameNode>(existingID);
			script->AddEditorContext<SPrintEntityNameNodeEditorContext>(node->UID);
			return node;
		}
}
}
