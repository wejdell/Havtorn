// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	// Represents a color in RGBA 0-255 range.
	struct HAVTORN_API SColor
	{
		static const F32 F32Max;
		static const F32 F32Min;
		static const F32 U8MaxAsF32;
		static const F32 U8MaxAsF32Reciprocal; 

		U8 R, G, B, A = 255;

		static const SColor Red;
		static const SColor Green;
		static const SColor Blue;
		static const SColor Black;
		static const SColor White;
		static const SColor Grey;
		static const SColor Teal;
		static const SColor Orange;
		static const SColor Magenta;
		static const SColor Yellow;
		// TODO.AG: Add more color presets

		inline SColor();
		// Alpha is set to 255.
		inline SColor(U8 monochrome);
		inline SColor(U8 r, U8 g, U8 b);
		inline SColor(U8 r, U8 g, U8 b, U8 a);
		// Expects float values to be [0.0f - 1.0f] range.
		inline SColor(const float r, const float g, const float b, const float a);
		// Expects SVector values to be [0.0f - 1.0f] range.
		inline SColor(const SVector& rgb);
		// Expects SVector4 values to be [0.0f - 1.0f] range.
		inline SColor(const SVector4& rgba);

		// Returns the RGB values in a [0.0f - 1.0f] range.
		inline SVector AsVector() const;
		// Returns the RGBA values in a [0.0f - 1.0f] range.
		inline SVector4 AsVector4() const;

		// Expects SVector values to be [0.0f - 1.0f] range.
		inline static U8 ToU8Range(const F32 c);
		// Returs color value in a [0.0f - 1.0f] range.
		inline static F32 ToFloatRange(const U8 c);
		inline static SColor Random(U8 lowerBound, U8 upperBound, U8 alpha);
		inline static SColor Random(U8 lowerBound, U8 upperBound);
		inline static SColor RandomGrey(U8 lowerBound, U8 upperBound, U8 alpha);
	};

	SColor::SColor() {}

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

	inline SColor::SColor(const float r, const float g, const float b, const float a)
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
		return static_cast<U8>(UMath::Clamp(c, F32Min, F32Max) * U8MaxAsF32);
	}

	F32 SColor::ToFloatRange(const U8 c)
	{
		return static_cast<F32>(c) * U8MaxAsF32Reciprocal;
	}

	inline SColor SColor::Random(U8 lowerBound, U8 upperBound, U8 alpha)
	{
		return SColor(
			static_cast<U8>(UMath::Random(lowerBound, upperBound)), 
			static_cast<U8>(UMath::Random(lowerBound, upperBound)), 
			static_cast<U8>(UMath::Random(lowerBound, upperBound)), 
			alpha);
	}

	inline SColor SColor::Random(U8 lowerBound, U8 upperBound)
	{
		return SColor::Random(lowerBound,upperBound, static_cast<U8>(UMath::Random(lowerBound, upperBound)));
	}

	inline SColor SColor::RandomGrey(U8 lowerBound, U8 upperBound, U8 alpha)
	{
		U8 c = static_cast<U8>(UMath::Random(lowerBound, upperBound));
		return SColor(c, c, c, alpha);
	}
}
