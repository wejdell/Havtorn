// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "../HexRune.h"

//
// Branch
// Sequence
// math operators
// <
// >
// <=
// >=
// ==
// !=
// math nodes, trig, vector/matrix operations
// for loop
// print string
// append string
// timer/delay
// 
// BeginPlay
// Tick
// StopPlay
//

namespace Havtorn
{
	namespace HexRune
	{
		struct SDataBindingGetNode : public SNode
		{
			ENGINE_API SDataBindingGetNode(const U64 id, const U32 typeID, SScript* owningScript, const U64 dataBindingID);
			virtual ENGINE_API I8 OnExecute() override;
			U64 DataBindingID = 0;
		};

		struct SDataBindingSetNode : public SNode
		{
			ENGINE_API SDataBindingSetNode(const U64 id, const U32 typeID, SScript* owningScript, const U64 dataBindingID);
			virtual ENGINE_API I8 OnExecute() override;
			U64 DataBindingID = 0;
		};

		struct SBranchNode : public SNode
		{
			ENGINE_API SBranchNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SSequenceNode : public SNode
		{
			ENGINE_API SSequenceNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SDelayNode : public SNode
		{
			ENGINE_API SDelayNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SBeginPlayNode : public SNode
		{
			ENGINE_API SBeginPlayNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct STickNode : public SNode
		{
			ENGINE_API STickNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SEndPlayNode : public SNode
		{
			ENGINE_API SEndPlayNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SPrintStringNode : public SNode
		{
			ENGINE_API SPrintStringNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SAppendStringNode : public SNode
		{
			ENGINE_API SAppendStringNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SFloatLessThanNode : public SNode
		{
			ENGINE_API SFloatLessThanNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SFloatMoreThanNode : public SNode
		{
			ENGINE_API SFloatMoreThanNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SFloatLessOrEqualNode : public SNode
		{
			ENGINE_API SFloatLessOrEqualNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SFloatMoreOrEqualNode : public SNode
		{
			ENGINE_API SFloatMoreOrEqualNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SFloatEqualNode : public SNode
		{
			ENGINE_API SFloatEqualNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SFloatNotEqualNode : public SNode
		{
			ENGINE_API SFloatNotEqualNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SIntLessThanNode : public SNode
		{
			ENGINE_API SIntLessThanNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SIntMoreThanNode : public SNode
		{
			ENGINE_API SIntMoreThanNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SIntLessOrEqualNode : public SNode
		{
			ENGINE_API SIntLessOrEqualNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SIntMoreOrEqualNode : public SNode
		{
			ENGINE_API SIntMoreOrEqualNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SIntEqualNode : public SNode
		{
			ENGINE_API SIntEqualNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SIntNotEqualNode : public SNode
		{
			ENGINE_API SIntNotEqualNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};
	}
}