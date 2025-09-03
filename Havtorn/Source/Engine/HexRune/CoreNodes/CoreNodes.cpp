// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "CoreNodes.h"
#include "ECS/GUIDManager.h"

namespace Havtorn
{
	namespace HexRune
	{
		SDataBindingGetNode::SDataBindingGetNode(const U64 id, SScript* owningScript, const U64 dataBindingID)
			: SNode::SNode(id, owningScript, ENodeType::EDataBindingGetNode)
		{
			DataBinding = &(*std::ranges::find_if(OwningScript->DataBindings, [dataBindingID](SScriptDataBinding& binding) { return binding.UID == dataBindingID; }));
			AddOutput(UGUIDManager::Generate(), DataBinding->Type, DataBinding->Name);
		}

		I8 SDataBindingGetNode::OnExecute()
		{
			SetDataOnPin(EPinDirection::Output, 0, DataBinding->Data);
			return 0;
		}

		SDataBindingSetNode::SDataBindingSetNode(const U64 id, SScript* owningScript, const U64 dataBindingID)
			: SNode::SNode(id, owningScript, ENodeType::EDataBindingSetNode)
		{
			DataBinding = &(*std::ranges::find_if(OwningScript->DataBindings, [dataBindingID](SScriptDataBinding& binding) { return binding.UID == dataBindingID; }));
			AddInput(UGUIDManager::Generate(), DataBinding->Type, DataBinding->Name);
		}

		I8 SDataBindingSetNode::OnExecute()
		{
			if (Inputs[0].IsDataUnset())
				return 0;

			DataBinding->Data = Inputs[0].Data;
			return 0;
		}

		SBranchNode::SBranchNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EBranchNode)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow);
			AddInput(UGUIDManager::Generate(), EPinType::Bool, "Condition");

			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "True");
			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "False");
		}

		I8 SBranchNode::OnExecute()
		{
			bool condition = false;
			GetDataOnPin(EPinDirection::Input, 0, condition);

			return condition ? 0 : 1;
		}

		SSequenceNode::SSequenceNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::ESequenceNode)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow);

			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "0");
			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "1");
			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "2");
		}

		I8 SSequenceNode::OnExecute()
		{
			return -1;
		}

		SDelayNode::SDelayNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EDelayNode)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow);
			AddInput(UGUIDManager::Generate(), EPinType::Float, "Duration");

			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "Completed");
		}

		I8 SDelayNode::OnExecute()
		{
			// TODO.NW: Timer manager?
			// OnTimer : return 0;

			return -2;
		}

		SBeginPlayNode::SBeginPlayNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EBeginPlayNode)
		{
			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
		}

		I8 SBeginPlayNode::OnExecute()
		{
			return 0;
		}

		STickNode::STickNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::ETickNode)
		{
			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
			AddOutput(UGUIDManager::Generate(), EPinType::Float, "Dt");
		}

		I8 STickNode::OnExecute()
		{
			SetDataOnPin(EPinDirection::Output, 1, GTime::Dt());
			return 0;
		}

		SEndPlayNode::SEndPlayNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EEndPlayNode)
		{
			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
			// TODO.NW: End play reason?
		}

		I8 SEndPlayNode::OnExecute()
		{
			// SetDataOnPin(EPinDirection::Output, 1, EndPlayReason);
			return 0;
		}

		SPrintStringNode::SPrintStringNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EPrintStringNode)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow);
			AddInput(UGUIDManager::Generate(), EPinType::String, "String");
			// TODO.NW: log category

			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
		}

		I8 SPrintStringNode::OnExecute()
		{
			std::string output = "";
			GetDataOnPin(EPinDirection::Input, 1, output);
			HV_LOG_INFO("%s", output.c_str());

			return 0;
		}

		SAppendStringNode::SAppendStringNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EAppendStringNode)
		{
			FlowType = EFlowType::Simple;
			AddInput(UGUIDManager::Generate(), EPinType::String, "A");
			AddInput(UGUIDManager::Generate(), EPinType::String, "B");
			AddInput(UGUIDManager::Generate(), EPinType::String, "C");

			AddOutput(UGUIDManager::Generate(), EPinType::String, "Return");
		}

		I8 SAppendStringNode::OnExecute()
		{
			std::string a, b, c = "";
			GetDataOnPin(EPinDirection::Input, 0, a);
			GetDataOnPin(EPinDirection::Input, 1, b);
			GetDataOnPin(EPinDirection::Input, 2, c);

			SetDataOnPin(EPinDirection::Output, 0, a.append(b.append(c)));
			return -1;
		}

		SFloatLessThanNode::SFloatLessThanNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EFloatLessThanNode)
		{
			FlowType = EFlowType::Simple;
			AddInput(UGUIDManager::Generate(), EPinType::Float);
			AddInput(UGUIDManager::Generate(), EPinType::Float);

			AddOutput(UGUIDManager::Generate(), EPinType::Bool);
		}

		I8 SFloatLessThanNode::OnExecute()
		{
			F32 a, b = 0.0f;
			GetDataOnPin(EPinDirection::Input, 0, a);
			GetDataOnPin(EPinDirection::Input, 1, b);
			SetDataOnPin(EPinDirection::Output, 0, a < b);
			return -1;
		}

		SFloatMoreThanNode::SFloatMoreThanNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EFloatMoreThanNode)
		{
			FlowType = EFlowType::Simple;
			AddInput(UGUIDManager::Generate(), EPinType::Float);
			AddInput(UGUIDManager::Generate(), EPinType::Float);

			AddOutput(UGUIDManager::Generate(), EPinType::Bool);
		}

		I8 SFloatMoreThanNode::OnExecute()
		{
			F32 a, b = 0.0f;
			GetDataOnPin(EPinDirection::Input, 0, a);
			GetDataOnPin(EPinDirection::Input, 1, b);
			SetDataOnPin(EPinDirection::Output, 0, a > b);
			return -1;
		}

		SFloatLessOrEqualNode::SFloatLessOrEqualNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EFloatLessOrEqualNode)
		{
			FlowType = EFlowType::Simple;
			AddInput(UGUIDManager::Generate(), EPinType::Float);
			AddInput(UGUIDManager::Generate(), EPinType::Float);

			AddOutput(UGUIDManager::Generate(), EPinType::Bool);
		}

		I8 SFloatLessOrEqualNode::OnExecute()
		{
			F32 a, b = 0.0f;
			GetDataOnPin(EPinDirection::Input, 0, a);
			GetDataOnPin(EPinDirection::Input, 1, b);
			SetDataOnPin(EPinDirection::Output, 0, a <= b);
			return -1;
		}

		SFloatMoreOrEqualNode::SFloatMoreOrEqualNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EFloatMoreOrEqualNode)
		{
			FlowType = EFlowType::Simple;
			AddInput(UGUIDManager::Generate(), EPinType::Float);
			AddInput(UGUIDManager::Generate(), EPinType::Float);

			AddOutput(UGUIDManager::Generate(), EPinType::Bool);
		}

		I8 SFloatMoreOrEqualNode::OnExecute()
		{
			F32 a, b = 0.0f;
			GetDataOnPin(EPinDirection::Input, 0, a);
			GetDataOnPin(EPinDirection::Input, 1, b);
			SetDataOnPin(EPinDirection::Output, 0, a >= b);
			return -1;
		}

		SFloatEqualNode::SFloatEqualNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EFloatEqualNode)
		{
			FlowType = EFlowType::Simple;
			AddInput(UGUIDManager::Generate(), EPinType::Float);
			AddInput(UGUIDManager::Generate(), EPinType::Float);

			AddOutput(UGUIDManager::Generate(), EPinType::Bool);
		}

		I8 SFloatEqualNode::OnExecute()
		{
			F32 a, b = 0.0f;
			GetDataOnPin(EPinDirection::Input, 0, a);
			GetDataOnPin(EPinDirection::Input, 1, b);
			SetDataOnPin(EPinDirection::Output, 0, UMath::NearlyEqual(a, b));
			return -1;
		}

		SFloatNotEqualNode::SFloatNotEqualNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EFloatNotEqualNode)
		{
			FlowType = EFlowType::Simple;
			AddInput(UGUIDManager::Generate(), EPinType::Float);
			AddInput(UGUIDManager::Generate(), EPinType::Float);

			AddOutput(UGUIDManager::Generate(), EPinType::Bool);
		}

		I8 SFloatNotEqualNode::OnExecute()
		{
			F32 a, b = 0.0f;
			GetDataOnPin(EPinDirection::Input, 0, a);
			GetDataOnPin(EPinDirection::Input, 1, b);
			SetDataOnPin(EPinDirection::Output, 0, a != b);
			return -1;
		}

		SIntLessThanNode::SIntLessThanNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EIntLessThanNode)
		{
			FlowType = EFlowType::Simple;
			AddInput(UGUIDManager::Generate(), EPinType::Int);
			AddInput(UGUIDManager::Generate(), EPinType::Int);

			AddOutput(UGUIDManager::Generate(), EPinType::Bool);
		}

		I8 SIntLessThanNode::OnExecute()
		{
			I32 a, b = 0;
			GetDataOnPin(EPinDirection::Input, 0, a);
			GetDataOnPin(EPinDirection::Input, 1, b);
			SetDataOnPin(EPinDirection::Output, 0, a < b);
			return -1;
		}

		SIntMoreThanNode::SIntMoreThanNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EIntMoreThanNode)
		{
			FlowType = EFlowType::Simple;
			AddInput(UGUIDManager::Generate(), EPinType::Int);
			AddInput(UGUIDManager::Generate(), EPinType::Int);

			AddOutput(UGUIDManager::Generate(), EPinType::Bool);
		}

		I8 SIntMoreThanNode::OnExecute()
		{
			I32 a, b = 0;
			GetDataOnPin(EPinDirection::Input, 0, a);
			GetDataOnPin(EPinDirection::Input, 1, b);
			SetDataOnPin(EPinDirection::Output, 0, a > b);
			return -1;
		}

		SIntLessOrEqualNode::SIntLessOrEqualNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EIntLessOrEqualNode)
		{
			FlowType = EFlowType::Simple;
			AddInput(UGUIDManager::Generate(), EPinType::Int);
			AddInput(UGUIDManager::Generate(), EPinType::Int);

			AddOutput(UGUIDManager::Generate(), EPinType::Bool);
		}

		I8 SIntLessOrEqualNode::OnExecute()
		{
			I32 a, b = 0;
			GetDataOnPin(EPinDirection::Input, 0, a);
			GetDataOnPin(EPinDirection::Input, 1, b);
			SetDataOnPin(EPinDirection::Output, 0, a <= b);
			return -1;
		}

		SIntMoreOrEqualNode::SIntMoreOrEqualNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EIntMoreOrEqualNode)
		{
			FlowType = EFlowType::Simple;
			AddInput(UGUIDManager::Generate(), EPinType::Int);
			AddInput(UGUIDManager::Generate(), EPinType::Int);

			AddOutput(UGUIDManager::Generate(), EPinType::Bool);
		}

		I8 SIntMoreOrEqualNode::OnExecute()
		{
			I32 a, b = 0;
			GetDataOnPin(EPinDirection::Input, 0, a);
			GetDataOnPin(EPinDirection::Input, 1, b);
			SetDataOnPin(EPinDirection::Output, 0, a >= b);
			return -1;
		}

		SIntEqualNode::SIntEqualNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EIntEqualNode)
		{
			FlowType = EFlowType::Simple;
			AddInput(UGUIDManager::Generate(), EPinType::Int);
			AddInput(UGUIDManager::Generate(), EPinType::Int);

			AddOutput(UGUIDManager::Generate(), EPinType::Bool);
		}

		I8 SIntEqualNode::OnExecute()
		{
			I32 a, b = 0;
			GetDataOnPin(EPinDirection::Input, 0, a);
			GetDataOnPin(EPinDirection::Input, 1, b);
			SetDataOnPin(EPinDirection::Output, 0, a == b);
			return -1;
		}

		SIntNotEqualNode::SIntNotEqualNode(const U64 id, SScript* owningScript)
			: SNode::SNode(id, owningScript, ENodeType::EIntNotEqualNode)
		{
			FlowType = EFlowType::Simple;
			AddInput(UGUIDManager::Generate(), EPinType::Int);
			AddInput(UGUIDManager::Generate(), EPinType::Int);

			AddOutput(UGUIDManager::Generate(), EPinType::Bool);
		}

		I8 SIntNotEqualNode::OnExecute()
		{
			I32 a, b = 0;
			GetDataOnPin(EPinDirection::Input, 0, a);
			GetDataOnPin(EPinDirection::Input, 1, b);
			SetDataOnPin(EPinDirection::Output, 0, a != b);
			return -1;
		}
	}
}
