// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

struct ImVec4;

namespace Havtorn
{
	// A set of static functions for conversion between Havtorn and ImGui types.
	class UImGuiTypeUtil
	{
	public:
		inline static ImVec2 Vector2ToImVec2(const SVector2<F32>& v);
		inline static ImVec4 VectorToImVec4(const SVector& v, const F32 w);
		inline static ImVec4 Vector4ToImVec4(const SVector4& v);

	public:
		inline static SVector2<F32> ImVec2ToVector2(const ImVec2& v);
		inline static SVector ImVec4ToVector(const ImVec4& v);
		inline static SVector4 ImVec4ToVector4(const ImVec4& v);
	};
}
