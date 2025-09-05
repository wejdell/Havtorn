// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "../HexRune.h"

namespace Havtorn
{
	namespace HexRune
	{
		struct SEntityLoopNode : public SNode
		{
			ENGINE_API SEntityLoopNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SComponentLoopNode : public SNode
		{
			ENGINE_API SComponentLoopNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SOnBeginOverlapNode : public SNode
		{
			ENGINE_API SOnBeginOverlapNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SOnEndOverlapNode : public SNode
		{
			ENGINE_API SOnEndOverlapNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SPrintEntityNameNode : public SNode
		{
			ENGINE_API SPrintEntityNameNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct SSetStaticMeshNode : public SNode
		{
			ENGINE_API SSetStaticMeshNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};

		struct STogglePointLightNode : public SNode
		{
			ENGINE_API STogglePointLightNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual ENGINE_API I8 OnExecute() override;
		};
	}
}