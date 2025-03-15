// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	struct SVector;
	struct SVector4;

	// Represents a color in RGBA 0-255 range.
	struct CORE_API SColor
	{
		static const F32 F32Max;
		static const F32 F32Min;
		static const F32 U8MaxAsF32;
		static const F32 U8MaxAsF32Reciprocal; 
		
		U8 R = 255; 
		U8 G = 255; 
		U8 B = 255;
		U8 A = 255;

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

		SColor() = default;
		// Alpha is set to 255.
		SColor(U8 monochrome);
		SColor(U8 r, U8 g, U8 b);
		SColor(U8 r, U8 g, U8 b, U8 a);
		// Expects float values to be [0.0f - 1.0f] range.
		SColor(const float r, const float g, const float b, const float a);
		// Expects SVector values to be [0.0f - 1.0f] range.
		SColor(const SVector& rgb);
		// Expects SVector4 values to be [0.0f - 1.0f] range.
		SColor(const SVector4& rgba);

		// TODO.NR: Add to/from HEX utilities

		// Returns the RGB values in a [0.0f - 1.0f] range.
		SVector AsVector() const;
		// Returns the RGBA values in a [0.0f - 1.0f] range.
		SVector4 AsVector4() const;

		// Expects SVector values to be [0.0f - 1.0f] range.
		static U8 ToU8Range(const F32 c);
		// Returs color value in a [0.0f - 1.0f] range.
		static F32 ToFloatRange(const U8 c);
		static SColor Random(U8 lowerBound, U8 upperBound, U8 alpha);
		static SColor Random(U8 lowerBound, U8 upperBound);
		static SColor RandomGrey(U8 lowerBound, U8 upperBound, U8 alpha);

		static SColor FromPackedU32(const U32 packed);
		static U32 ToPackedU32(const SColor& color);

		auto operator<=>(const SColor& other) const = default;
	};
}
