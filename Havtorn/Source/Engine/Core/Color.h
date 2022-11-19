// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	// Represents a color in RGBA 0-255 range.
	struct HAVTORN_API SColor
	{
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
		// Expects SVector values to be [0.0f - 1.0f] range.
		inline SColor(const SVector& rgb);
		// Expects SVector4 values to be [0.0f - 1.0f] range.
		inline SColor(const SVector4& rgba);

		// Returns the RGB values in a [0.0f - 1.0f] range.
		inline SVector AsVector() const;
		// Returns the RGBA values in a [0.0f - 1.0f] range.
		inline SVector4 AsVector4() const;
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

	SColor::SColor(const SVector& rgb)
		: R(static_cast<U8>(rgb.X * 255.0f))
		, G(static_cast<U8>(rgb.Y * 255.0f))
		, B(static_cast<U8>(rgb.Z * 255.0f))
	{}

	SColor::SColor(const SVector4& rgba)
		: R(static_cast<U8>(rgba.X * 255.0f))
		, G(static_cast<U8>(rgba.Y * 255.0f))
		, B(static_cast<U8>(rgba.Z * 255.0f))
		, A(static_cast<U8>(rgba.W * 255.0f))
	{}

	SVector SColor::AsVector() const
	{
		SVector v;
		v.X = static_cast<F32>(R) / 255.0f;
		v.Y = static_cast<F32>(G) / 255.0f;
		v.Z = static_cast<F32>(B) / 255.0f;
		return v;
	}

	SVector4 SColor::AsVector4() const
	{
		SVector4 v;
		v.X = static_cast<F32>(R) / 255.0f;
		v.Y = static_cast<F32>(G) / 255.0f;
		v.Z = static_cast<F32>(B) / 255.0f;
		v.W = static_cast<F32>(A) / 255.0f;
		return v;
	}
}
