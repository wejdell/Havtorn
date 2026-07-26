// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "../NodeView.h"

namespace Havtorn
{
	namespace HexRune
	{
		struct SDataBindingGetNodeEditorContext : public SNodeView
		{
			SDataBindingGetNodeEditorContext(SScript* script, const U64 dataBindingID);

			U64 DataBindingID = 0;
		};

		struct SDataBindingSetNodeEditorContext : public SNodeView
		{
			SDataBindingSetNodeEditorContext(SScript* script, const U64 dataBindingID);

			U64 DataBindingID = 0;
		};

		struct SBranchNodeEditorContext : public SNodeView
		{
			SBranchNodeEditorContext();
		};

		struct SSequenceNodeEditorContext : public SNodeView
		{
			SSequenceNodeEditorContext();
		};

		struct SDelayNodeEditorContext : public SNodeView
		{
			SDelayNodeEditorContext();
		};

		struct SBeginPlayNodeEditorContext : public SNodeView
		{
			SBeginPlayNodeEditorContext();
		};

		struct STickNodeEditorContext : public SNodeView
		{
			STickNodeEditorContext();
		};

		struct SEndPlayNodeEditorContext : public SNodeView
		{
			SEndPlayNodeEditorContext();
		};

		struct SPrintStringNodeEditorContext : public SNodeView
		{
			SPrintStringNodeEditorContext();
		};

		struct SAppendStringNodeEditorContext : public SNodeView
		{
			SAppendStringNodeEditorContext();
		};

		struct SFloatLessThanNodeEditorContext : public SNodeView
		{
			SFloatLessThanNodeEditorContext();
		};

		struct SFloatMoreThanNodeEditorContext : public SNodeView
		{
			SFloatMoreThanNodeEditorContext();
		};

		struct SFloatLessOrEqualNodeEditorContext : public SNodeView
		{
			SFloatLessOrEqualNodeEditorContext();
		};

		struct SFloatMoreOrEqualNodeEditorContext : public SNodeView
		{
			SFloatMoreOrEqualNodeEditorContext();
		};

		struct SFloatEqualNodeEditorContext : public SNodeView
		{
			SFloatEqualNodeEditorContext();
		};

		struct SFloatNotEqualNodeEditorContext : public SNodeView
		{
			SFloatNotEqualNodeEditorContext();
		};

		struct SIntLessThanNodeEditorContext : public SNodeView
		{
			SIntLessThanNodeEditorContext();
		};

		struct SIntMoreThanNodeEditorContext : public SNodeView
		{
			SIntMoreThanNodeEditorContext();
		};

		struct SIntLessOrEqualNodeEditorContext : public SNodeView
		{
			SIntLessOrEqualNodeEditorContext();
		};

		struct SIntMoreOrEqualNodeEditorContext : public SNodeView
		{
			SIntMoreOrEqualNodeEditorContext();
		};

		struct SIntEqualNodeEditorContext : public SNodeView
		{
			SIntEqualNodeEditorContext();
		};

		struct SIntNotEqualNodeEditorContext : public SNodeView
		{
			SIntNotEqualNodeEditorContext();
		};
	}
}
