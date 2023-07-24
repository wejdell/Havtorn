// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "GraphicsStructs.h"

namespace Havtorn
{
	namespace GeometryPrimitives
	{
		static std::vector<SStaticMeshVertex> Cube = 
		{
			// X      Y      Z        nX, nY, nZ    tX, tY, tZ,    bX, bY, bZ,    UV	
			{ -0.5f, -0.5f, -0.5f,   -1,  0,  0,    0,  0,  1,     0,  1,  0,     0, 0 }, // 0
			{  0.5f, -0.5f, -0.5f,    1,  0,  0,    0,  0, -1,     0,  1,  0,     1, 0 }, // 1
			{ -0.5f,  0.5f, -0.5f,   -1,  0,  0,    0,  0,  1,     0,  1,  0,     0, 1 }, // 2
			{  0.5f,  0.5f, -0.5f,    1,  0,  0,    0,  0, -1,     0,  1,  0,     1, 1 }, // 3
			{ -0.5f, -0.5f,  0.5f,   -1,  0,  0,    0,  0,  1,     0,  1,  0,     0, 0 }, // 4
			{  0.5f, -0.5f,  0.5f,    1,  0,  0,    0,  0, -1,     0,  1,  0,     1, 0 }, // 5
			{ -0.5f,  0.5f,  0.5f,   -1,  0,  0,    0,  0,  1,     0,  1,  0,     0, 1 }, // 6
			{  0.5f,  0.5f,  0.5f,    1,  0,  0,    0,  0, -1,     0,  1,  0,     1, 1 }, // 7
			// X      Y      Z        nX, nY, nZ    nX, nY, nZ,    nX, nY, nZ,    UV	  
			{ -0.5f, -0.5f, -0.5f,    0, -1,  0,    1,  0,  0,     0,  0,  1,     0, 0 }, // 8  // 0
			{  0.5f, -0.5f, -0.5f,    0, -1,  0,    1,  0,  0,     0,  0,  1,     1, 0 }, // 9	// 1
			{ -0.5f,  0.5f, -0.5f,    0,  1,  0,   -1,  0,  0,     0,  0,  1,     0, 0 }, // 10	// 2
			{  0.5f,  0.5f, -0.5f,    0,  1,  0,   -1,  0,  0,     0,  0,  1,     1, 0 }, // 11	// 3
			{ -0.5f, -0.5f,  0.5f,    0, -1,  0,    1,  0,  0,     0,  0,  1,     0, 1 }, // 12	// 4
			{  0.5f, -0.5f,  0.5f,    0, -1,  0,    1,  0,  0,     0,  0,  1,     0, 1 }, // 13	// 5
			{ -0.5f,  0.5f,  0.5f,    0,  1,  0,   -1,  0,  0,     0,  0,  1,     1, 1 }, // 14	// 6
			{  0.5f,  0.5f,  0.5f,    0,  1,  0,   -1,  0,  0,     0,  0,  1,     1, 1 }, // 15	// 7
			// X      Y      Z        nX, nY, nZ    nX, nY, nZ,    nX, nY, nZ,    UV	  
			{ -0.5f, -0.5f, -0.5f,    0,  0, -1,   -1,  0,  0,     0,  1,  0,     0, 0 }, // 16 // 0
			{  0.5f, -0.5f, -0.5f,    0,  0, -1,   -1,  0,  0,     0,  1,  0,     0, 0 }, // 17	// 1
			{ -0.5f,  0.5f, -0.5f,    0,  0, -1,   -1,  0,  0,     0,  1,  0,     1, 0 }, // 18	// 2
			{  0.5f,  0.5f, -0.5f,    0,  0, -1,   -1,  0,  0,     0,  1,  0,     1, 0 }, // 19	// 3
			{ -0.5f, -0.5f,  0.5f,    0,  0,  1,    1,  0,  0,     0,  1,  0,     0, 1 }, // 20	// 4
			{  0.5f, -0.5f,  0.5f,    0,  0,  1,    1,  0,  0,     0,  1,  0,     1, 1 }, // 21	// 5
			{ -0.5f,  0.5f,  0.5f,    0,  0,  1,    1,  0,  0,     0,  1,  0,     0, 1 }, // 22	// 6
			{  0.5f,  0.5f,  0.5f,    0,  0,  1,    1,  0,  0,     0,  1,  0,     1, 1 }  // 23	// 7
		};

		static std::vector<U32> CubeIndices =
		{
			0,4,2,
			4,6,2,
			1,3,5,
			3,7,5,
			8,9,12,
			9,13,12,
			10,14,11,
			14,15,11,
			16,18,17,
			18,19,17,
			20,21,22,
			21,23,22
		};

		static std::vector<SPositionVertex> PointLightCube = 
		{
			// X      Y      Z      W 
			{ -1.0f, -1.0f, -1.0f,  1.0f },// 0
			{  1.0f, -1.0f, -1.0f,  1.0f },// 1
			{ -1.0f,  1.0f, -1.0f,  1.0f },// 2
			{  1.0f,  1.0f, -1.0f,  1.0f },// 3
			{ -1.0f, -1.0f,  1.0f,  1.0f },// 4
			{  1.0f, -1.0f,  1.0f,  1.0f },// 5
			{ -1.0f,  1.0f,  1.0f,  1.0f },// 6
			{  1.0f,  1.0f,  1.0f,  1.0f }//  7
		};

		static std::vector<U32> PointLightCubeIndices = 
		{
			0,4,2,
			4,6,2,
			1,3,5,
			3,7,5,
			0,1,4,
			1,5,4,
			2,6,3,
			6,7,3,
			0,2,1,
			2,3,1,
			4,5,6,
			5,7,6
		};

		static std::vector<SStaticMeshVertex> DecalProjector =
		{
			// X      Y      Z        nX, nY, nZ    tX, tY, tZ    bX, bY, bZ,    UV	
			{ -0.5f, -0.5f, -0.5f,    0,  0, -1,    1,  0,  0,    0,  -1,  0,     0, 0 },
			{  0.5f, -0.5f, -0.5f,    0,  0, -1,    1,  0,  0,    0,  -1,  0,     1, 0 },
			{ -0.5f,  0.5f, -0.5f,    0,  0, -1,    1,  0,  0,    0,  -1,  0,     0, 1 },
			{  0.5f,  0.5f, -0.5f,    0,  0, -1,    1,  0,  0,    0,  -1,  0,     1, 1 },
			{ -0.5f, -0.5f,  0.5f,    0,  0, -1,    1,  0,  0,    0,  -1,  0,     0, 0 },
			{  0.5f, -0.5f,  0.5f,    0,  0, -1,    1,  0,  0,    0,  -1,  0,     1, 0 },
			{ -0.5f,  0.5f,  0.5f,    0,  0, -1,    1,  0,  0,    0,  -1,  0,     0, 1 },
			{  0.5f,  0.5f,  0.5f,    0,  0, -1,    1,  0,  0,    0,  -1,  0,     1, 1 }
		};

		static std::vector<U32> DecalProjectorIndices = 
		{
			0,4,2,
			4,6,2,
			1,3,5,
			3,7,5,
			0,1,4,
			1,5,4,
			2,6,3,
			6,7,3,
			0,2,1,
			2,3,1,
			4,5,6,
			5,7,6
		};

		static U32 GetNewVertex(U32 i1, U32 i2, std::vector<SVector>& outPositions, std::map<std::tuple<U32, U32>, U32>& newVertices)
		{
			std::tuple<U32, U32> t1(i1, i2);
			std::tuple<U32, U32> t2(i2, i1);
			
			if (newVertices.contains(t2)) 
			{
				return newVertices[t2];
			}
			if (newVertices.contains(t1)) 
			{
				return newVertices[t1];
			}

			U32 newIndex = static_cast<U32>(outPositions.size());
			newVertices.emplace(t1, newIndex);
			outPositions.emplace_back((outPositions[i1] + outPositions[i2]) * 0.5f );
			return newIndex;
		};

		static void Subdivide(std::vector<SVector>& outPositions, std::vector<U32>& outIndices)
		{
			std::vector<U32> indices;
			std::map<std::tuple<U32, U32>, U32> newVertices;

			U32 numTris = static_cast<U32>(outIndices.size()) / 3;

			for (U32 i = 0; i < numTris; i++) 
			{
				//       i2
				//       *
				//      / \
				//     /   \
				//   a*-----*b
				//   / \   / \
				//  /   \ /   \
				// *-----*-----*
				// i1    c      i3

				U32 i1 = outIndices[i * 3];
				U32 i2 = outIndices[i * 3 + 1];
				U32 i3 = outIndices[i * 3 + 2];
				U32 a = GetNewVertex(i1, i2, outPositions, newVertices);
				U32 b = GetNewVertex(i2, i3, outPositions, newVertices);
				U32 c = GetNewVertex(i3, i1, outPositions, newVertices);
				
				indices.emplace_back(i1);
				indices.emplace_back(a);
				indices.emplace_back(c);
				indices.emplace_back(i2);
				indices.emplace_back(b);
				indices.emplace_back(a);
				indices.emplace_back(i3);
				indices.emplace_back(c);
				indices.emplace_back(b);
				indices.emplace_back(a);
				indices.emplace_back(b);
				indices.emplace_back(c);
			}

			outIndices = indices;
		}

		static SMeshData CreateIcosphere(U8 numberOfSubdivisions)
		{
			// NR: Adapted from Introduction to 3D Graphics Programming with Direct X 12 by Frank D. Luna

			// Put a cap on the number of subdivisions.
			numberOfSubdivisions = UMath::Min(numberOfSubdivisions, static_cast<U8>(6));

			// NR: We use a world transform to scale this instead of providing a radius argument here
			std::vector<SStaticMeshVertex> vertices;

			// Approximate a sphere by tessellating an icosahedron.
			const float X = 0.525731f;
			const float Z = 0.850651f;

			std::vector<SVector> positions =
			{
				SVector(-X, 0.0f, Z),	SVector(X, 0.0f, Z),
				SVector(-X, 0.0f, -Z),	SVector(X, 0.0f, -Z),
				SVector(0.0f, Z, X),	SVector(0.0f, Z, -X),
				SVector(0.0f, -Z, X),	SVector(0.0f, -Z, -X),
				SVector(Z, X, 0.0f),	SVector(-Z, X, 0.0f),
				SVector(Z, -X, 0.0f),	SVector(-Z, -X, 0.0f)
			};

			std::vector<U32> indices =
			{
				1,4,0, 4,9,0, 4,5,9, 8,5,4, 1,8,4,
				1,10,8, 10,3,8, 8,3,5, 3,2,5, 3,7,2,
				3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
				10,1,6, 11,0,9, 2,11,9, 5,2,9, 11,2,7
			};
			
			for (U32 i = 0; i < numberOfSubdivisions; ++i)
				Subdivide(positions, indices);

			// Project vertices onto sphere and scale.
			for (U16 i = 0; i < static_cast<U16>(positions.size()); ++i)
			{
				// Project onto unit sphere.
				positions[i].Normalize();
				SVector normal = positions[i];

				// Derive texture coordinates from spherical coordinates.
				F32 theta = UMath::ATan2(positions[i].Z, positions[i].X);
				
				// Put in [0, 2pi].
				if (theta < 0.0f)
					theta += UMath::Tau;

				F32 phi = UMath::ACos(positions[i].Y);

				SVector2<F32> uv;
				uv.X = theta * UMath::TauReciprocal;
				uv.Y = phi * UMath::PiReciprocal;

				SVector tangent;
				// Partial derivative of P with respect to theta
				tangent.X = -1.0f * UMath::Sin(phi) * UMath::Sin(theta);
				tangent.Y = 0.0f;
				tangent.Z = UMath::Sin(phi) * UMath::Cos(theta);
				
				tangent.Normalize();

				SVector bitangent = normal.Cross(tangent);

				vertices.push_back({ positions[i].X, positions[i].Y, positions[i].Z, normal.X, normal.Y, normal.Z, tangent.X, tangent.Y, tangent.Z, bitangent.X, bitangent.Y, bitangent.Z, uv.X, uv.Y });
			}

			return {vertices, indices};
		}

		static SMeshData Icosphere = CreateIcosphere(2);

#pragma region SPRIMITIVES_FOR_DEBUG_SHAPES

		/*
			[Last edited: 2022-12-01 by AG]
			SPrimitives are currently only used by DebugShape-Component/System. Max limit on vertices and indices are U8: 255.
		*/

		const static SPrimitive Line = 
		{
			{
				{ 0.0f, 0.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 1.0f, 1.0f }
			},
			{
				0, 1
			}
		};
		
		const static SPrimitive Pyramid =
		{
			{
				{ 0.0f, 1.0f, 0.0f, 1.0f },
				{ 0.5f, 0.0f, 0.5f, 1.0f },
				{ -0.5f, 0.0f, 0.5f, 1.0f },
				{ -0.5f, 0.0f, -0.5f, 1.0f },
				{ 0.5f, 0.0f, -0.5f, 1.0f },
			},
			{
				0,1, 0,2, 0,3, 0,4,
				1,2, 2,3, 3,4, 4,1
			}
		};

		const static SPrimitive BoundingBox =
		{
			{
				// X      Y      Z      W 
				{ -0.5f, -0.5f, -0.5f,  1.0f },// 0
				{  0.5f, -0.5f, -0.5f,  1.0f },// 1
				{ -0.5f,  0.5f, -0.5f,  1.0f },// 2
				{  0.5f,  0.5f, -0.5f,  1.0f },// 3
				{ -0.5f, -0.5f,  0.5f,  1.0f },// 4
				{  0.5f, -0.5f,  0.5f,  1.0f },// 5
				{ -0.5f,  0.5f,  0.5f,  1.0f },// 6
				{  0.5f,  0.5f,  0.5f,  1.0f },// 7
			},
			{
				0, 1, 1, 3, 3, 2, 2, 0, 0, 4, 4, 6, 6, 2, 6, 7, 7, 5, 5, 4, 7, 3, 5, 1,
			}

		};

		const static SPrimitive Camera =
		{
			{
				// X      Y      Z      W
				{ 0.0f, 0.0f, 0.0f, 1.0f },// 0
				{ 0.3f, 0.3f, 0.7f, 1.0f },// 1
				{ 0.3f, -0.3f, 0.7f, 1.0f },// 2
				{ -0.3f, 0.3f, 0.7f, 1.0f },// 3
				{ -0.3f, -0.3f, 0.7f, 1.0f },// 4
				{ -0.15f, 0.325f, 0.7f, 1.0f },// 5
				{ 0.0f, 0.45f, 0.7f, 1.0f },// 6
				{ 0.15f, 0.325f, 0.7f, 1.0f },// 7
			},
			{
				0, 1, 0, 2, 0, 3, 0, 4,
				// Far plane
				1, 2, 2, 4, 4, 3, 3, 1,
				// Arrow above far plane.
				5, 6, 6, 7, 7, 5 
			}
		};

		constexpr F32 CircleRadius = 0.5f;

		// Creates circle across the XZ-plane
		const static std::vector<SPositionVertex> CircleVertices(const F32 maxRadians, const F32 radius, U32 segments)
		{
			segments = UMath::Max(4u, static_cast<U32>(UMath::DecrementUntilEven(segments)));

			const F32 step = maxRadians / static_cast<F32>(segments);

			std::vector<SPositionVertex> vertices;
			for (U32 i = 0u; i < segments; i++)
			{
				SPositionVertex point(
					UMath::Cos(step * static_cast<F32>(i)) * radius,
					0.0f,
					UMath::Sin(step * static_cast<F32>(i)) * radius,
					1.0f
				);
				vertices.push_back(point);
			}

			return vertices;
		}
		
		const static std::vector<U32> CircleIndicesLineTopology(U32 segments)
		{
			segments = UMath::Max(4u, static_cast<U32>(UMath::DecrementUntilEven(segments)));

			std::vector<U32> indices;
			for (U32 i = 0u; i < segments; i++)
			{
				U32 next = i + 1u;
				next %= segments;
				indices.push_back(i);
				indices.push_back(next);
			};
			return indices;
		}

		const static std::vector<SPositionVertex> HalfCircleVertices(const F32 maxRadians, const F32 radius, U32 segments)
		{
			std::vector<SPositionVertex> vertices = CircleVertices(maxRadians, radius, segments);
			SPositionVertex v;
			v.x = -vertices[0].x;
			v.y = vertices[0].y;
			v.z = vertices[0].z;
			v.w = vertices[0].w;
			vertices.push_back(v);
			return vertices;
		}

		const static std::vector<U32> HalfCircleIndicesLineTopology(U32 segments)
		{
			std::vector<U32> indices = CircleIndicesLineTopology(segments);
			// Remove indices composing line between last and first.
			indices.pop_back();
			indices.pop_back();

			segments = UMath::Max(4u, static_cast<U32>(UMath::DecrementUntilEven(segments)));
			// Account for odd vertex added in HalfCircleVertices
			indices.push_back(segments - 1u);
			indices.push_back(segments);
			return indices;
		}

		// 8-segment circle across XZ-plane
		const static SPrimitive Circle8 =
		{
			CircleVertices(UMath::Tau, CircleRadius, 8u),
			CircleIndicesLineTopology(8u)
		};

		// 16-segment circle across XZ-plane
		const static SPrimitive Circle16 =
		{
			CircleVertices(UMath::Tau, CircleRadius, 16u),
			CircleIndicesLineTopology(16u)
		};

		// 32-segment circle across XZ-plane
		const static SPrimitive Circle32 =
		{
			CircleVertices(UMath::Tau, CircleRadius, 32u),
			CircleIndicesLineTopology(32u)
		};

		const static SPrimitive HalfCircle16 =
		{
			HalfCircleVertices(UMath::Pi, CircleRadius, 17u),
			HalfCircleIndicesLineTopology(17u)
		};
		
		const static SPrimitive GenerateGrid(U64 segments)
		{
			const U64 segmentsFinal = segments + 1;

			SPrimitive grid;
			grid.Vertices.reserve(segmentsFinal * 4);
			grid.Indices.resize(segmentsFinal * 4);

			SPositionVertex v;
			v.y = 0.0f;
			v.w = 1.0f;
			const F32 start = static_cast<F32>(segments / 2);
			for (U64 i = 0; i < segments + 1; i++)
			{
				v.x = -start + static_cast<F32>(i);
				v.z = -start;
				grid.Vertices.push_back(v);
				v.z = start;
				grid.Vertices.push_back(v);

				v.x = -start;
				v.z = -start + static_cast<F32>(i);
				grid.Vertices.push_back(v);
				v.x = start;
				grid.Vertices.push_back(v);
			}

			for (U64 i = 0; i < (segments + 1) * 4; i++)
			{
				grid.Indices[i] = static_cast<U32>(i);
			}
			grid.Vertices.shrink_to_fit();
			return grid;
		}

		// 10x10 segment across XZ-plane.
		const static SPrimitive Grid = GenerateGrid(10);

		const static SPrimitive Axis =
		{
			{
				{ 0.5f,		0.0f,	0.0f,	 1.0f},
				{ -0.5f,	0.0f,	0.0f,	 1.0f},
				{ 0.0f,		0.5f,	0.0f,	 1.0f},
				{ 0.0f,		-0.5f,	0.0f,	 1.0f},
				{ 0.0f,		0.0f,	0.5f,	 1.0f},
				{ 0.0f,		0.0f,	-0.5f,	 1.0f},
			},
			{
				0,1, 2,3, 4,5
			}
		};

		// TODO.AG: Make less expensive to draw.
		const static SPrimitive Octahedron =
		{
			{ 
				{ 0.0f, 0.5f, 0.0f, 1.0f },
				{ 0.5f, 0.0f, 0.0f, 1.0f },
				{ -0.5f, 0.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 0.5f, 1.0f },
				{ 0.0f, 0.0f, -0.5f, 1.0f },
				{ 0.0f, -0.5f, 0.0f, 1.0f },
			},
			{
				0,1, 0,2, 0,3, 0,4, 
				5,1, 5,2, 5,3, 5,4, 
				1,4, 4,2, 2,3, 3,1 
			}
		};

		const static SPrimitive Square =
		{
			{
				{  0.5f,  0.0f,  0.5f,  1.0f },
				{ -0.5f,  0.0f,  0.5f,  1.0f },
				{ -0.5f,  0.0f, -0.5f,  1.0f },
				{  0.5f,  0.0f, -0.5f,  1.0f },
			},
			{
				0,1, 1,2, 2,3, 3,0 
			}
		};

		static std::vector<SPositionVertex> UVSphereVertices(const F32 diameter, U32 latitudes, U32 longitudes)
		{
			// AG: Modified version of: https://gist.github.com/Pikachuxxxx/5c4c490a7d7679824e0e18af42918efc
			
			longitudes = UMath::Max(3u, longitudes);
			latitudes = UMath::Max(2u, latitudes);

			const F32 theta = 2.0f * UMath::Pi;
			const F32 phi = UMath::Pi;
			F32 deltaLatitude = phi / static_cast<F32>(latitudes);
			F32 deltaLongitude = theta / static_cast<F32>(longitudes);
			F32 latitudeAngle;
			F32 longitudeAngle;

			std::vector<SPositionVertex> vertices;
			
			// Add North and South poles to indices 0 & 1 respectively.
			vertices.push_back({0.0f, diameter, 0.0f, 1.0f});
			vertices.push_back({0.0f, -diameter, 0.0f, 1.0f});
			
			const F32 halfPi = UMath::Pi / 2.0f;
			// [ 1 >= i <= latitudes - 1 ]:
			// NPole == latitudeAngle = halfPi - (0 * deltaLatitude);
			// SPole == latitudeAngle = halfPi - (latitudes * deltaLatitude);
			for (U32 i = 1u; i <= (latitudes - 1u); i++)
			{
				// Every step of latitudeAngle creates 1 circle around the Y axis. => Indices 2, ..., 2+longitudes-1 shape 1 circle
				latitudeAngle = halfPi - (static_cast<F32>(i) * deltaLatitude);
				F32 xz = diameter * UMath::Cos(latitudeAngle);
				F32 y = diameter * UMath::Sin(latitudeAngle);

				for (U32 j = 0u; j < longitudes; j++)
				{
					longitudeAngle = static_cast<F32>(j) * deltaLongitude;

					SPositionVertex vertex;
					vertex.x = xz * UMath::Cos(longitudeAngle);
					vertex.y = y;
					vertex.z = xz * UMath::Sin(longitudeAngle);
					vertex.w = 1.0f;
					vertices.push_back(vertex);
				}
			}

			return vertices;			
		}

		static std::vector<U32> UVSphereIndicesLineTopo(U32 latitudes, U32 longitudes)
		{
			longitudes = UMath::Max(3u, longitudes);
			latitudes = UMath::Max(2u, latitudes);

			std::vector<U32> indices;
			
			// Latitudinal indices: 
			U32 startIndex = 2;
			for (U32 i = 0; i <= (latitudes - 2); i++)
			{
				U32 endIndex = startIndex + (longitudes - 1);
				indices.push_back(startIndex);
				indices.push_back(endIndex);
				for (U32 j = startIndex; j < endIndex;)
				{
					U32 nextIndex = j + 1;
					indices.push_back(j);
					indices.push_back(nextIndex);
					j = nextIndex;
				}
				startIndex = endIndex + 1;
			}

			// Longitudinal indices: 
			const U32 northPole = 0;
			const U32 southPole = 1;
			startIndex = 2;
			for (U32 i = 0; i < longitudes; i++)
			{
				U32 endIndex = startIndex + ((latitudes - 2) * longitudes);
				indices.push_back(northPole);
				indices.push_back(startIndex);
				indices.push_back(endIndex);
				indices.push_back(southPole);
				for (U32 j = startIndex; j < endIndex;)
				{
					U32 nextIndex = j + (longitudes);
					indices.push_back(j);
					indices.push_back(nextIndex);
					j = nextIndex;
				}
				startIndex++;
			}

			return indices;
		}

		const static SPrimitive UVSphere =
		{
			UVSphereVertices(1.0f, 12, 12),
			UVSphereIndicesLineTopo(12, 12)
		};

#pragma endregion !SPRIMITIVES_FOR_DEBUG_SHAPES
	}
}
