// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "CoreNodes.h"
#include "ECS/GUIDManager.h"

namespace Havtorn
{
	namespace HexRune
	{
		SDataBindingGetNode::SDataBindingGetNode(const U64 id, const U32 typeID, SScript* owningScript, const U64 dataBindingID)
			: SNode::SNode(id, typeID, owningScript, ENodeType::DataBindingGetNode)
			, DataBindingID(dataBindingID)
		{
			SScriptDataBinding* databinding = &(*std::ranges::find_if(OwningScript->DataBindings, [dataBindingID](SScriptDataBinding& binding) { return binding.UID == dataBindingID; }));
			AddOutput(UGUIDManager::Generate(), databinding->Type, databinding->Name);
		}

		I8 SDataBindingGetNode::OnExecute()
		{
			SScriptDataBinding* databinding = &(*std::ranges::find_if(OwningScript->DataBindings, [&](SScriptDataBinding& binding) { return binding.UID == DataBindingID; }));
			SetDataOnPin(EPinDirection::Output, 0, databinding->Data);
			return 0;
		}

		SDataBindingSetNode::SDataBindingSetNode(const U64 id, const U32 typeID, SScript* owningScript, const U64 dataBindingID)
			: SNode::SNode(id, typeID, owningScript, ENodeType::DataBindingSetNode)
			, DataBindingID(dataBindingID)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow);

			SScriptDataBinding* databinding = &(*std::ranges::find_if(OwningScript->DataBindings, [dataBindingID](SScriptDataBinding& binding) { return binding.UID == dataBindingID; }));
			AddInput(UGUIDManager::Generate(), databinding->Type, databinding->Name);

			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
		}

		I8 SDataBindingSetNode::OnExecute()
		{
			if (Inputs[1].IsDataUnset())
				return 0;

			SScriptDataBinding* databinding = &(*std::ranges::find_if(OwningScript->DataBindings, [&](SScriptDataBinding& binding) { return binding.UID == DataBindingID; }));
			databinding->Data = Inputs[1].Data;
			return 0;
		}

		SBranchNode::SBranchNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SSequenceNode::SSequenceNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SDelayNode::SDelayNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SBeginPlayNode::SBeginPlayNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
		}

		I8 SBeginPlayNode::OnExecute()
		{
			return 0;
		}

		STickNode::STickNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
			AddOutput(UGUIDManager::Generate(), EPinType::Float, "Dt");
		}

		I8 STickNode::OnExecute()
		{
			SetDataOnPin(EPinDirection::Output, 1, GTime::Dt());
			return 0;
		}

		SEndPlayNode::SEndPlayNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
			// TODO.NW: End play reason?
		}

		I8 SEndPlayNode::OnExecute()
		{
			// SetDataOnPin(EPinDirection::Output, 1, EndPlayReason);
			return 0;
		}

		SPrintStringNode::SPrintStringNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow);
			AddInput(UGUIDManager::Generate(), EPinType::String, "String");
			AddInput(UGUIDManager::Generate(), EPinType::Float, "Float");

			// TODO.NW: log category

			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
		}

		I8 SPrintStringNode::OnExecute()
		{
			std::string output = "";
			GetDataOnPin(EPinDirection::Input, 1, output);
			F32 outputFloat = 0;
			GetDataOnPin(EPinDirection::Input, 2, outputFloat);

			HV_LOG_INFO("%s", output.c_str());
			HV_LOG_INFO("%f", outputFloat);

			return 0;
		}

		SAppendStringNode::SAppendStringNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SFloatLessThanNode::SFloatLessThanNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SFloatMoreThanNode::SFloatMoreThanNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SFloatLessOrEqualNode::SFloatLessOrEqualNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SFloatMoreOrEqualNode::SFloatMoreOrEqualNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SFloatEqualNode::SFloatEqualNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SFloatNotEqualNode::SFloatNotEqualNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SIntLessThanNode::SIntLessThanNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SIntMoreThanNode::SIntMoreThanNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SIntLessOrEqualNode::SIntLessOrEqualNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SIntMoreOrEqualNode::SIntMoreOrEqualNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SIntEqualNode::SIntEqualNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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

		SIntNotEqualNode::SIntNotEqualNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
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
