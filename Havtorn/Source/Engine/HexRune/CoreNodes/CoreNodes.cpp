// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "CoreNodes.h"
#include "ECS/GUIDManager.h"

namespace Havtorn
{
	namespace HexRune
	{
		void SBranchNode::Construct()
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

		void SSequenceNode::Construct()
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

		void SEntityLoopNode::Construct()
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow);
			AddInput(UGUIDManager::Generate(), EPinType::ObjectArray, "Array");
			
			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "Loop Body");
			AddOutput(UGUIDManager::Generate(), EPinType::Object, "Element");
			AddOutput(UGUIDManager::Generate(), EPinType::Int, "Index");
			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "Completed");
		}

		I8 SEntityLoopNode::OnExecute()
		{
			std::vector<SEntity> entities;
			GetDataOnPin(EPinDirection::Input, 0, entities);
			
			for (I32 i = 0; i < STATIC_I32(entities.size()); i++)
			{
				SEntity& entity = entities[i];
				SetDataOnPin(EPinDirection::Output, 1, entity);
				SetDataOnPin(EPinDirection::Output, 2, i);

				if (Outputs[0].LinkedPin != nullptr)
					Outputs[0].LinkedPin->OwningNode->Execute();
			}

			return 3;
		}

		void SComponentLoopNode::Construct()
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow);
			AddInput(UGUIDManager::Generate(), EPinType::ObjectArray, "Array");

			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "Loop Body");
			AddOutput(UGUIDManager::Generate(), EPinType::Object, "Element");
			AddOutput(UGUIDManager::Generate(), EPinType::Int, "Index");
			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "Completed");
		}

		I8 SComponentLoopNode::OnExecute()
		{
			std::vector<SComponent*> components;
			GetDataOnPin(EPinDirection::Input, 0, components);

			for (I32 i = 0; i < STATIC_I32(components.size()); i++)
			{
				SComponent* component = components[i];
				SetDataOnPin(EPinDirection::Output, 1, component);
				SetDataOnPin(EPinDirection::Output, 2, i);

				if (Outputs[0].LinkedPin != nullptr)
					Outputs[0].LinkedPin->OwningNode->Execute();
			}

			return 3;
		}

		void SDelayNode::Construct()
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

		void SBeginPlayNode::Construct()
		{
			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
		}

		I8 SBeginPlayNode::OnExecute()
		{
			return 0;
		}

		void STickNode::Construct()
		{
			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
			AddOutput(UGUIDManager::Generate(), EPinType::Float, "Dt");
		}

		I8 STickNode::OnExecute()
		{
			SetDataOnPin(EPinDirection::Output, 1, GTime::Dt());
			return 0;
		}

		void SEndPlayNode::Construct()
		{
			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
			// TODO.NW: End play reason?
		}

		I8 SEndPlayNode::OnExecute()
		{
			// SetDataOnPin(EPinDirection::Output, 1, EndPlayReason);
			return 0;
		}

		void SPrintStringNode::Construct()
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

		void SAppendStringNode::Construct()
		{
			Type = ENodeType::Simple;
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

		void SFloatLessThanNode::Construct()
		{
			Type = ENodeType::Simple;
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

		void SFloatMoreThanNode::Construct()
		{
			Type = ENodeType::Simple;
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

		void SFloatLessOrEqualNode::Construct()
		{
			Type = ENodeType::Simple;
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

		void SFloatMoreOrEqualNode::Construct()
		{
			Type = ENodeType::Simple;
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

		void SFloatEqualNode::Construct()
		{
			Type = ENodeType::Simple;
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

		void SFloatNotEqualNode::Construct()
		{
			Type = ENodeType::Simple;
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

		void SIntLessThanNode::Construct()
		{
			Type = ENodeType::Simple;
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

		void SIntMoreThanNode::Construct()
		{
			Type = ENodeType::Simple;
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

		void SIntLessOrEqualNode::Construct()
		{
			Type = ENodeType::Simple;
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

		void SIntMoreOrEqualNode::Construct()
		{
			Type = ENodeType::Simple;
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

		void SIntEqualNode::Construct()
		{
			Type = ENodeType::Simple;
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

		void SIntNotEqualNode::Construct()
		{
			Type = ENodeType::Simple;
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