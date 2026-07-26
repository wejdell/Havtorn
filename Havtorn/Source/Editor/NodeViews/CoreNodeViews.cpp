// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CoreNodeViews.h"

#include <HexRune/HexRune.h>

#include <GUI.h>

namespace Havtorn
{
	namespace HexRune
	{
		SDataBindingGetNodeEditorContext::SDataBindingGetNodeEditorContext(SScript* script, const U64 dataBindingID)
			: DataBindingID(dataBindingID)
		{
			auto it = &(*std::ranges::find_if(script->DataBindings, [dataBindingID](SScriptDataBinding& binding) { return binding.UID == dataBindingID; }));
			Name = "Get " + it->Name;
			Category = "Data Bindings";
			Color = SColor::Orange;
		}

		SDataBindingSetNodeEditorContext::SDataBindingSetNodeEditorContext(SScript* script, const U64 dataBindingID)
			: DataBindingID(dataBindingID)
		{
			auto it = &(*std::ranges::find_if(script->DataBindings, [dataBindingID](SScriptDataBinding& binding) { return binding.UID == dataBindingID; }));
			Name = "Set " + it->Name;
			Category = "Data Bindings";
			Color = SColor::Orange;
		}

		SBranchNodeEditorContext::SBranchNodeEditorContext()
		{
			Name = "Branch";
			Category = "General";
		}

		SSequenceNodeEditorContext::SSequenceNodeEditorContext()
		{
			Name = "Sequence";
			Category = "General";
		}

		SDelayNodeEditorContext::SDelayNodeEditorContext()
		{
			Name = "Delay";
			Category = "General";
			Color = SColor::Teal;
		}

		SBeginPlayNodeEditorContext::SBeginPlayNodeEditorContext()
		{
			Name = "Begin Play";
			Category = "General";
			Color = SColor::Red;
		}

		STickNodeEditorContext::STickNodeEditorContext()
		{
			Name = "Tick";
			Category = "General";
			Color = SColor::Red;
		}

		SEndPlayNodeEditorContext::SEndPlayNodeEditorContext()
		{
			Name = "End Play";
			Category = "General";
			Color = SColor::Red;
		}

		SPrintStringNodeEditorContext::SPrintStringNodeEditorContext()
		{
			Name = "Print String";
			Category = "General";
			Color = SColor::Teal;
		}

		SAppendStringNodeEditorContext::SAppendStringNodeEditorContext()
		{
			Name = "Append String";
			Category = "General";
			Color = SColor::Green;
		}

		SFloatLessThanNodeEditorContext::SFloatLessThanNodeEditorContext()
		{
			Name = "< (Float)";
			Category = "Math";
		}

		SFloatMoreThanNodeEditorContext::SFloatMoreThanNodeEditorContext()
		{
			Name = "> (Float)";
			Category = "Math";
		}

		SFloatLessOrEqualNodeEditorContext::SFloatLessOrEqualNodeEditorContext()
		{
			Name = "<= (Float)";
			Category = "Math";
		}

		SFloatMoreOrEqualNodeEditorContext::SFloatMoreOrEqualNodeEditorContext()
		{
			Name = ">= (Float)";
			Category = "Math";
		}

		SFloatEqualNodeEditorContext::SFloatEqualNodeEditorContext()
		{
			Name = "== (Float)";
			Category = "Math";
		}

		SFloatNotEqualNodeEditorContext::SFloatNotEqualNodeEditorContext()
		{
			Name = "!= (Float)";
			Category = "Math";
		}

		SIntLessThanNodeEditorContext::SIntLessThanNodeEditorContext()
		{
			Name = "< (Int)";
			Category = "Math";
		}

		SIntMoreThanNodeEditorContext::SIntMoreThanNodeEditorContext()
		{
			Name = "> (Int)";
			Category = "Math";
		}

		SIntLessOrEqualNodeEditorContext::SIntLessOrEqualNodeEditorContext()
		{
			Name = "<= (Int)";
			Category = "Math";
		}

		SIntMoreOrEqualNodeEditorContext::SIntMoreOrEqualNodeEditorContext()
		{
			Name = ">= (Int)";
			Category = "Math";
		}

		SIntEqualNodeEditorContext::SIntEqualNodeEditorContext()
		{
			Name = "== (Int)";
			Category = "Math";
		}

		SIntNotEqualNodeEditorContext::SIntNotEqualNodeEditorContext()
		{
			Name = "!= (Int)";
			Category = "Math";
		}
	}
}
