// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "EngineMath.h"
#include <limits>

namespace Havtorn
{
	HAVTORN_API const F32 UMath::Pi = 3.14159265359f;
	HAVTORN_API const F32 UMath::Tau = 6.28318530718f;

	HAVTORN_API const F32 UMath::PiReciprocal = 0.31830988618f;
	HAVTORN_API const F32 UMath::TauReciprocal = 0.15915494309f;

	HAVTORN_API const F32 UMath::MinFloat = (std::numeric_limits<F32>::min)();
	HAVTORN_API const F32 UMath::MaxFloat = (std::numeric_limits<F32>::max)();

	HAVTORN_API const U8 UMath::MinU8 = (std::numeric_limits<U8>::min)();
	HAVTORN_API const U8 UMath::MaxU8 = (std::numeric_limits<U8>::max)();
}
