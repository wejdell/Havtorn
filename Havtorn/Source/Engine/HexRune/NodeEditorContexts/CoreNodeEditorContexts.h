// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "../NodeEditorContext.h"

namespace Havtorn
{
	namespace HexRune
	{
		struct SDataBindingNodeEditorContext : public SNodeEditorContext
		{
			SDataBindingNodeEditorContext(SScript* script, const U64 dataBindingID);
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;

			// NW: Note lack of static context for data bindings, as they are registered in the script per data binding

			U64 DataBindingID = 0;
		};

		struct SBranchNodeEditorContext : public SNodeEditorContext
		{
			SBranchNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SBranchNodeEditorContext Context;
		};

		struct SSequenceNodeEditorContext : public SNodeEditorContext
		{
			SSequenceNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SSequenceNodeEditorContext Context;
		};

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

		struct SDelayNodeEditorContext : public SNodeEditorContext
		{
			SDelayNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SDelayNodeEditorContext Context;
		};

		struct SBeginPlayNodeEditorContext : public SNodeEditorContext
		{
			SBeginPlayNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SBeginPlayNodeEditorContext Context;
		};

		struct STickNodeEditorContext : public SNodeEditorContext
		{
			STickNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static STickNodeEditorContext Context;
		};

		struct SEndPlayNodeEditorContext : public SNodeEditorContext
		{
			SEndPlayNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SEndPlayNodeEditorContext Context;
		};

		struct SPrintStringNodeEditorContext : public SNodeEditorContext
		{
			SPrintStringNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SPrintStringNodeEditorContext Context;
		};

		struct SAppendStringNodeEditorContext : public SNodeEditorContext
		{
			SAppendStringNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SAppendStringNodeEditorContext Context;
		};

		struct SFloatLessThanNodeEditorContext : public SNodeEditorContext
		{
			SFloatLessThanNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SFloatLessThanNodeEditorContext Context;
		};

		struct SFloatMoreThanNodeEditorContext : public SNodeEditorContext
		{
			SFloatMoreThanNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SFloatMoreThanNodeEditorContext Context;
		};

		struct SFloatLessOrEqualNodeEditorContext : public SNodeEditorContext
		{
			SFloatLessOrEqualNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SFloatLessOrEqualNodeEditorContext Context;
		};

		struct SFloatMoreOrEqualNodeEditorContext : public SNodeEditorContext
		{
			SFloatMoreOrEqualNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SFloatMoreOrEqualNodeEditorContext Context;
		};

		struct SFloatEqualNodeEditorContext : public SNodeEditorContext
		{
			SFloatEqualNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SFloatEqualNodeEditorContext Context;
		};

		struct SFloatNotEqualNodeEditorContext : public SNodeEditorContext
		{
			SFloatNotEqualNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SFloatNotEqualNodeEditorContext Context;
		};

		struct SIntLessThanNodeEditorContext : public SNodeEditorContext
		{
			SIntLessThanNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SIntLessThanNodeEditorContext Context;
		};

		struct SIntMoreThanNodeEditorContext : public SNodeEditorContext
		{
			SIntMoreThanNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SIntMoreThanNodeEditorContext Context;
		};

		struct SIntLessOrEqualNodeEditorContext : public SNodeEditorContext
		{
			SIntLessOrEqualNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SIntLessOrEqualNodeEditorContext Context;
		};

		struct SIntMoreOrEqualNodeEditorContext : public SNodeEditorContext
		{
			SIntMoreOrEqualNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SIntMoreOrEqualNodeEditorContext Context;
		};

		struct SIntEqualNodeEditorContext : public SNodeEditorContext
		{
			SIntEqualNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SIntEqualNodeEditorContext Context;
		};

		struct SIntNotEqualNodeEditorContext : public SNodeEditorContext
		{
			SIntNotEqualNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SIntNotEqualNodeEditorContext Context;
		};

		struct SPrintEntityNameNodeEditorContext : public SNodeEditorContext
		{
			SPrintEntityNameNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SPrintEntityNameNodeEditorContext Context;
		};

	}
}
