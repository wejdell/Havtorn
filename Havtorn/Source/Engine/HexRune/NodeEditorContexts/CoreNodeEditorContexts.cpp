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
		SDataBindingGetNodeEditorContext::SDataBindingGetNodeEditorContext(SScript* script, const U64 dataBindingID)
			: DataBindingID(dataBindingID)
		{
			auto it = &(*std::ranges::find_if(script->DataBindings, [dataBindingID](SScriptDataBinding& binding) { return binding.UID == dataBindingID; }));
			Name = "Get " + it->Name;
			Category = "Data Bindings";
			Color = SColor::Orange;
		}

		SNode* SDataBindingGetNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SDataBindingGetNode>(existingID, TypeID, DataBindingID);
			script->AddEditorContext<SDataBindingGetNodeEditorContext>(node->UID, script, DataBindingID);
			return node;
		}

		SDataBindingSetNodeEditorContext::SDataBindingSetNodeEditorContext(SScript* script, const U64 dataBindingID)
			: DataBindingID(dataBindingID)
		{
			auto it = &(*std::ranges::find_if(script->DataBindings, [dataBindingID](SScriptDataBinding& binding) { return binding.UID == dataBindingID; }));
			Name = "Set " + it->Name;
			Category = "Data Bindings";
			Color = SColor::Orange;
		}
		
		SNode* SDataBindingSetNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SDataBindingSetNode>(existingID, TypeID, DataBindingID);
			script->AddEditorContext<SDataBindingSetNodeEditorContext>(node->UID, script, DataBindingID);
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

			SNode* node = script->AddNode<SBranchNode>(existingID, TypeID);
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

			SNode* node = script->AddNode<SSequenceNode>(existingID, TypeID);
			script->AddEditorContext<SSequenceNodeEditorContext>(node->UID);
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

			SNode* node = script->AddNode<SDelayNode>(existingID, TypeID);
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

		SNode* SBeginPlayNodeEditorContext::AddNode(SScript* script, const U64 /*existingID*/) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SBeginPlayNode>(BeginPlayNodeID, TypeID);
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

		SNode* STickNodeEditorContext::AddNode(SScript* script, const U64 /*existingID*/) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<STickNode>(TickNodeID, TypeID);
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

		SNode* SEndPlayNodeEditorContext::AddNode(SScript* script, const U64 /*existingID*/) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SEndPlayNode>(EndPlayNodeID, TypeID);
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

			SNode* node = script->AddNode<SPrintStringNode>(existingID, TypeID);
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

			SNode* node = script->AddNode<SAppendStringNode>(existingID, TypeID);
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

			SNode* node = script->AddNode<SFloatLessThanNode>(existingID, TypeID);
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

			SNode* node = script->AddNode<SFloatMoreThanNode>(existingID, TypeID);
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

			SNode* node = script->AddNode<SFloatLessOrEqualNode>(existingID, TypeID);
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

			SNode* node = script->AddNode<SFloatMoreOrEqualNode>(existingID, TypeID);
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

			SNode* node = script->AddNode<SFloatEqualNode>(existingID, TypeID);
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

			SNode* node = script->AddNode<SFloatNotEqualNode>(existingID, TypeID);
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

			SNode* node = script->AddNode<SIntLessThanNode>(existingID, TypeID);
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

			SNode* node = script->AddNode<SIntMoreThanNode>(existingID, TypeID);
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

			SNode* node = script->AddNode<SIntLessOrEqualNode>(existingID, TypeID);
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

			SNode* node = script->AddNode<SIntMoreOrEqualNode>(existingID, TypeID);
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

			SNode* node = script->AddNode<SIntEqualNode>(existingID, TypeID);
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

			SNode* node = script->AddNode<SIntNotEqualNode>(existingID, TypeID);
			script->AddEditorContext<SIntNotEqualNodeEditorContext>(node->UID);
			return node;
		}
	}
}
