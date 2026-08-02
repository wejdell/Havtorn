// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CoreNodeViews.h"

#include <HexRune/HexRune.h>

#include <GUI.h>

namespace Havtorn
{
	namespace HexRune
	{
		SDataBindingGetNodeView::SDataBindingGetNodeView(SScript* script, const U64 dataBindingID)
			: DataBindingID(dataBindingID)
		{
			auto it = &(*std::ranges::find_if(script->DataBindings, [dataBindingID](SScriptDataBinding& binding) { return binding.UID == dataBindingID; }));
			Name = "Get " + it->Name;
			Category = "Data Bindings";
			Color = SColor::Orange;
		}

		SDataBindingSetNodeView::SDataBindingSetNodeView(SScript* script, const U64 dataBindingID)
			: DataBindingID(dataBindingID)
		{
			auto it = &(*std::ranges::find_if(script->DataBindings, [dataBindingID](SScriptDataBinding& binding) { return binding.UID == dataBindingID; }));
			Name = "Set " + it->Name;
			Category = "Data Bindings";
			Color = SColor::Orange;
		}

		SBranchNodeView::SBranchNodeView()
		{
			Name = "Branch";
			Category = "General";
		}

		SSequenceNodeView::SSequenceNodeView()
		{
			Name = "Sequence";
			Category = "General";
		}

		SDelayNodeView::SDelayNodeView()
		{
			Name = "Delay";
			Category = "General";
			Color = SColor::Teal;
		}

		SBeginPlayNodeView::SBeginPlayNodeView()
		{
			Name = "Begin Play";
			Category = "General";
			Color = SColor::Red;
		}

		STickNodeView::STickNodeView()
		{
			Name = "Tick";
			Category = "General";
			Color = SColor::Red;
		}

		SEndPlayNodeView::SEndPlayNodeView()
		{
			Name = "End Play";
			Category = "General";
			Color = SColor::Red;
		}

		SPrintStringNodeView::SPrintStringNodeView()
		{
			Name = "Print String";
			Category = "General";
			Color = SColor::Teal;
		}

		SAppendStringNodeView::SAppendStringNodeView()
		{
			Name = "Append String";
			Category = "General";
			Color = SColor::Green;
		}

		SFloatLessThanNodeView::SFloatLessThanNodeView()
		{
			Name = "< (Float)";
			Category = "Math";
		}

		SFloatMoreThanNodeView::SFloatMoreThanNodeView()
		{
			Name = "> (Float)";
			Category = "Math";
		}

		SFloatLessOrEqualNodeView::SFloatLessOrEqualNodeView()
		{
			Name = "<= (Float)";
			Category = "Math";
		}

		SFloatMoreOrEqualNodeView::SFloatMoreOrEqualNodeView()
		{
			Name = ">= (Float)";
			Category = "Math";
		}

		SFloatEqualNodeView::SFloatEqualNodeView()
		{
			Name = "== (Float)";
			Category = "Math";
		}

		SFloatNotEqualNodeView::SFloatNotEqualNodeView()
		{
			Name = "!= (Float)";
			Category = "Math";
		}

		SIntLessThanNodeView::SIntLessThanNodeView()
		{
			Name = "< (Int)";
			Category = "Math";
		}

		SIntMoreThanNodeView::SIntMoreThanNodeView()
		{
			Name = "> (Int)";
			Category = "Math";
		}

		SIntLessOrEqualNodeView::SIntLessOrEqualNodeView()
		{
			Name = "<= (Int)";
			Category = "Math";
		}

		SIntMoreOrEqualNodeView::SIntMoreOrEqualNodeView()
		{
			Name = ">= (Int)";
			Category = "Math";
		}

		SIntEqualNodeView::SIntEqualNodeView()
		{
			Name = "== (Int)";
			Category = "Math";
		}

		SIntNotEqualNodeView::SIntNotEqualNodeView()
		{
			Name = "!= (Int)";
			Category = "Math";
		}
	}
}
