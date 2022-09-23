// Copyright 2022 Team Havtorn. All Rights Reserved.
#pragma once

#include "Graphics/GraphicsStructs.h"
#include "Core/CoreTypes.h"

#include <vector>
#include <array>

namespace Havtorn
{
	namespace Debug
	{
		struct SColor
		{
			F32 R, G, B, A;

			static const SColor Red;
			static const SColor Green;
			static const SColor Blue;
			static const SColor White;
		};

		struct SDebugShape
		{
			static constexpr U8 MAXVERTICES = 2;//32*32?
			std::array<SPositionVertex, MAXVERTICES> Vertices = {};
			//std::unique_ptr<SPositionVertex[]> Vertices = std::make_unique<SPositionVertex[]>(MAXVERTICES);// This works too
			U8 UsedVertices = 0;

			//std::vector<SPositionVertex> Vertices;
			
			SColor Color = SColor::White;
			F32 LifeTime = 0.0f;

			const bool operator<(const SDebugShape& rhs) const
			{
				return LifeTime < rhs.LifeTime;
			}

			const bool operator>(const SDebugShape& rhs) const
			{
				return LifeTime > rhs.LifeTime;
			}
		};

		/*
		*	Memory:
			Vertices:
			Sphere should be the most resource intensive shape, has the most vertices and indices.
			(vertical axis segments * horizontal axis segments * nr of floats * float in bytes)
			if max_segments = 32: 32 * 32 * 4 * 4 = 16 384 bytes ~ 16kb. 
			
			Color:
			nr of floats * floats = 4 * 4 = 16 bytes

			Time:
			1 float = 4 bytes

			Indicies (if/ when added):
			Count should be same or more as Vertices, largest shape should be sphere: 32 * 32 = 1024.
			U16 should be enough, U16 = unsigned 16bit value which can hold int values: 0 to 65 535.
			16bit is 2 bytes on a 16bit system. 
			2 bytes * 1024 = 2048 bytes.

			With MAX_SHAPES set to 10 000:
			Vertices * 10 000 
			+ Color * 10 000 
			+ Time * 10 000
			+ Indices * 10 000 
			=
			~160mb
			+ ~0.16mb
			+ ~0.04mb
			+ ~20.48mb
			=
			~180.68mb
		*/
	}
}