// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "hvpch.h"
#include <Core/imgui.h>
#include "ImGuiTypeUtil.h"

namespace Havtorn
{
	inline ImVec2 UImGuiTypeUtil::Vector2ToImVec2(const SVector2<F32>& v)
	{
		return ImVec2(v.X, v.Y);
	}
	inline ImVec4 UImGuiTypeUtil::VectorToImVec4(const SVector& v, const F32 w)
	{
		return ImVec4( v.X, v.Y, v.Z, w );
	}

	inline ImVec4 UImGuiTypeUtil::Vector4ToImVec4(const SVector4& v)
	{
		return ImVec4( v.X, v.Y, v.Z, v.W );
	}
}
