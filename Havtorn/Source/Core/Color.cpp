// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"

#include "Color.h"

namespace Havtorn
{
	const F32 SColor::F32Max = 1.0f;
	const F32 SColor::F32Min = 0.0f;
	const F32 SColor::U8MaxAsF32 = 255.0f;
	const F32 SColor::U8MaxAsF32Reciprocal = 1.0f / 255.0f;

	 const SColor SColor::Red = SColor(255, 0, 0);
	 const SColor SColor::Green = SColor(0, 255, 0);
	 const SColor SColor::Blue = SColor(0, 0, 255);
	 const SColor SColor::Black = SColor(0);
	 const SColor SColor::White = SColor(255);
	 const SColor SColor::Grey = SColor(127);
	 const SColor SColor::Teal = SColor(50, 255, 255);
	 const SColor SColor::Orange = SColor(255, 125, 15);
	 const SColor SColor::Magenta = SColor(200, 0, 225);
	 const SColor SColor::Yellow = SColor(255, 225, 25);

	 SColor::SColor(U8 monochrome)
		 : R(monochrome)
		 , G(monochrome)
		 , B(monochrome)
	 {}

	 SColor::SColor(U8 r, U8 g, U8 b)
		 : R(r)
		 , G(g)
		 , B(b)
	 {}

	 SColor::SColor(U8 r, U8 g, U8 b, U8 a)
		 : R(r)
		 , G(g)
		 , B(b)
		 , A(a)
	 {}

	 SColor::SColor(const float r, const float g, const float b, const float a)
		 : R(ToU8Range(r))
		 , G(ToU8Range(g))
		 , B(ToU8Range(b))
		 , A(ToU8Range(a))
	 {}

	 SColor::SColor(const SVector& rgb)
		 : R(ToU8Range(rgb.X))
		 , G(ToU8Range(rgb.Y))
		 , B(ToU8Range(rgb.Z))
	 {}

	 SColor::SColor(const SVector4& rgba)
		 : R(ToU8Range(rgba.X))
		 , G(ToU8Range(rgba.Y))
		 , B(ToU8Range(rgba.Z))
		 , A(ToU8Range(rgba.W))
	 {}

	 SVector SColor::AsVector() const
	 {
		 SVector v;
		 v.X = ToFloatRange(R);
		 v.Y = ToFloatRange(G);
		 v.Z = ToFloatRange(B);
		 return v;
	 }

	 SVector4 SColor::AsVector4() const
	 {
		 SVector4 v;
		 v.X = ToFloatRange(R);
		 v.Y = ToFloatRange(G);
		 v.Z = ToFloatRange(B);
		 v.W = ToFloatRange(A);
		 return v;
	 }

	 U8 SColor::ToU8Range(const F32 c)
	 {
		 return STATIC_U8(UMath::Clamp(c, F32Min, F32Max) * U8MaxAsF32);
	 }

	 F32 SColor::ToFloatRange(const U8 c)
	 {
		 return STATIC_F32(c) * U8MaxAsF32Reciprocal;
	 }

	 SColor SColor::Random(U8 lowerBound, U8 upperBound, U8 alpha)
	 {
		 return SColor(
			 STATIC_U8(UMath::Random(lowerBound, upperBound)),
			 STATIC_U8(UMath::Random(lowerBound, upperBound)),
			 STATIC_U8(UMath::Random(lowerBound, upperBound)),
			 alpha);
	 }

	 SColor SColor::Random(U8 lowerBound, U8 upperBound)
	 {
		 return SColor::Random(lowerBound, upperBound, STATIC_U8(UMath::Random(lowerBound, upperBound)));
	 }

	 SColor SColor::RandomGrey(U8 lowerBound, U8 upperBound, U8 alpha)
	 {
		 U8 c = STATIC_U8(UMath::Random(lowerBound, upperBound));
		 return SColor(c, c, c, alpha);
	 }

#define COL32_R_SHIFT    0
#define COL32_G_SHIFT    8
#define COL32_B_SHIFT    16
#define COL32_A_SHIFT    24
#define F32_TO_INT8_SAT(x)        ((I8)(UMath::Clamp(x) * 255.0f + 0.5f))               // Saturated, always output 0..255

	 SColor SColor::FromPackedU32(const U32 packed)
	 {
		 F32 scaling = 1.0f / 255.0f;
		 return SColor(
			 ((packed >> COL32_R_SHIFT) & 0xFF) * scaling,
			 ((packed >> COL32_G_SHIFT) & 0xFF) * scaling,
			 ((packed >> COL32_B_SHIFT) & 0xFF) * scaling,
			 ((packed >> COL32_A_SHIFT) & 0xFF) * scaling);
	 }

	 U32 SColor::ToPackedU32(const SColor& color)
	 {
		 SVector4 colorFloat = color.AsVector4();
		 U32 out;
		 out = ((U32)F32_TO_INT8_SAT(colorFloat.X)) << COL32_R_SHIFT;
		 out |= ((U32)F32_TO_INT8_SAT(colorFloat.Y)) << COL32_G_SHIFT;
		 out |= ((U32)F32_TO_INT8_SAT(colorFloat.Z)) << COL32_B_SHIFT;
		 out |= ((U32)F32_TO_INT8_SAT(colorFloat.W)) << COL32_A_SHIFT;
		 return out;
	 }
}