// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "../NodeView.h"

namespace Havtorn
{
	namespace HexRune
	{
		struct SDataBindingGetNodeView : public SNodeView
		{
			SDataBindingGetNodeView(SScript* script, const U64 dataBindingID);

			U64 DataBindingID = 0;
		};

		struct SDataBindingSetNodeView : public SNodeView
		{
			SDataBindingSetNodeView(SScript* script, const U64 dataBindingID);

			U64 DataBindingID = 0;
		};

		struct SBranchNodeView : public SNodeView
		{
			SBranchNodeView();
		};

		struct SSequenceNodeView : public SNodeView
		{
			SSequenceNodeView();
		};

		struct SDelayNodeView : public SNodeView
		{
			SDelayNodeView();
		};

		struct SBeginPlayNodeView : public SNodeView
		{
			SBeginPlayNodeView();
		};

		struct STickNodeView : public SNodeView
		{
			STickNodeView();
		};

		struct SEndPlayNodeView : public SNodeView
		{
			SEndPlayNodeView();
		};

		struct SPrintStringNodeView : public SNodeView
		{
			SPrintStringNodeView();
		};

		struct SAppendStringNodeView : public SNodeView
		{
			SAppendStringNodeView();
		};

		struct SFloatLessThanNodeView : public SNodeView
		{
			SFloatLessThanNodeView();
		};

		struct SFloatMoreThanNodeView : public SNodeView
		{
			SFloatMoreThanNodeView();
		};

		struct SFloatLessOrEqualNodeView : public SNodeView
		{
			SFloatLessOrEqualNodeView();
		};

		struct SFloatMoreOrEqualNodeView : public SNodeView
		{
			SFloatMoreOrEqualNodeView();
		};

		struct SFloatEqualNodeView : public SNodeView
		{
			SFloatEqualNodeView();
		};

		struct SFloatNotEqualNodeView : public SNodeView
		{
			SFloatNotEqualNodeView();
		};

		struct SIntLessThanNodeView : public SNodeView
		{
			SIntLessThanNodeView();
		};

		struct SIntMoreThanNodeView : public SNodeView
		{
			SIntMoreThanNodeView();
		};

		struct SIntLessOrEqualNodeView : public SNodeView
		{
			SIntLessOrEqualNodeView();
		};

		struct SIntMoreOrEqualNodeView : public SNodeView
		{
			SIntMoreOrEqualNodeView();
		};

		struct SIntEqualNodeView : public SNodeView
		{
			SIntEqualNodeView();
		};

		struct SIntNotEqualNodeView : public SNodeView
		{
			SIntNotEqualNodeView();
		};
	}
}
