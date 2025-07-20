// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "../NodeEditorContext.h"

namespace Havtorn
{
	namespace HexRune
	{
		struct SEntityLoopNodeEditorContext : public SNodeEditorContext
		{
			SEntityLoopNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SEntityLoopNodeEditorContext Context;
		};

		struct SComponentLoopNodeEditorContext : public SNodeEditorContext
		{
			SComponentLoopNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SComponentLoopNodeEditorContext Context;
		};

		struct SOnBeginOverlapNodeEditorContext : public SNodeEditorContext
		{
			SOnBeginOverlapNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SOnBeginOverlapNodeEditorContext Context;
		};

		struct SOnEndOverlapNodeEditorContext : public SNodeEditorContext
		{
			SOnEndOverlapNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SOnEndOverlapNodeEditorContext Context;
		};

		struct SPrintEntityNameNodeEditorContext : public SNodeEditorContext
		{
			SPrintEntityNameNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SPrintEntityNameNodeEditorContext Context;
		};

		struct SSetStaticMeshNodeEditorContext : public SNodeEditorContext
		{
			SSetStaticMeshNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SSetStaticMeshNodeEditorContext Context;
		};

		struct STogglePointLightNodeEditorContext : public SNodeEditorContext
		{
			STogglePointLightNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static STogglePointLightNodeEditorContext Context;
		};
	}
}
