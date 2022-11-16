// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Graphics/RenderManager.h"

namespace Havtorn
{
	namespace UGraphicsUtils
	{
		HAVTORN_API const size_t GetVertexCount(const EVertexBufferPrimitives primitive);

		HAVTORN_API const size_t GetIndexCount(const EVertexBufferPrimitives primitive);
	}
}
