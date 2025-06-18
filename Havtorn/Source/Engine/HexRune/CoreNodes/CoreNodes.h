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
// EndPlay
//

namespace Havtorn
{
	namespace HexRune
	{
		struct SBranchNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SSequenceNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SEntityLoopNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SComponentLoopNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SDelayNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SBeginPlayNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
			// TODO.NW Figure out if it's enough that the script System handles these
			virtual ENGINE_API bool IsStartNode() const override { return true; };
		};

		struct STickNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SEndPlayNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SPrintStringNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SAppendStringNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SFloatLessThanNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SFloatMoreThanNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SFloatLessOrEqualNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SFloatMoreOrEqualNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SFloatEqualNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SFloatNotEqualNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SIntLessThanNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SIntMoreThanNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SIntLessOrEqualNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SIntMoreOrEqualNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SIntEqualNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SIntNotEqualNode : public SNode
		{
			virtual ENGINE_API void Construct() override;
			virtual ENGINE_API I8 OnExecute() override;
		};
	}
}