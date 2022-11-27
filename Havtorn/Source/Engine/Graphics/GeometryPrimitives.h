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

			// NR: We use a world transform to scale this instead of providing a radius argument here
			
			std::vector<SStaticMeshVertex> vertices;

			// Put a cap on the number of subdivisions.
			numberOfSubdivisions = UMath::Min(numberOfSubdivisions, static_cast<U8>(6));
			
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

		const static std::vector<SPositionVertex> LineShape =
		{
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 1.0f, 1.0f }
		};

		const static std::vector<U32> LineShapeIndices =
		{
			0, 1, 0
		};
		
		const static std::vector<SPositionVertex> Arrow =
		{
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 1.0f, 1.0f },
			{ 0.15f, 0.0f, 0.8f, 1.0f },
			{ -0.15f, 0.0f, 0.8f, 1.0f },
			{ 0.0f, 0.15f, 0.8f, 1.0f },
			{ 0.0f, -0.15f, 0.8f, 1.0f },
		}; 

		const static std::vector<U32> ArrowIndices =
		{
			/*0, 1, 2, 1, 3, 1, 4, 1, 5, 1, 5, 3, 3, 4, 4, 1, 1, 5*/
			0, 1, 
			1, 2,
			1, 3,
			1, 4,
			1, 5,

			2, 3,
			4, 5,

			2, 4,
			4, 3,
			3, 5,
			5, 2

		};

		const static std::vector<SPositionVertex> DebugCube = 
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
		};

		const static std::vector<U32> DebugCubeIndices =
		{
			0, 1, 1, 3, 3, 2, 2, 0, 0, 4, 4, 6, 6, 2, 6, 7, 7, 5, 5, 4, 7, 3, 5, 1,
		};

		const static std::vector<SPositionVertex> Camera = 
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
		};

		const static std::vector<U32> CameraIndices =
		{
			0, 1, 0, 2, 0, 3, 0, 4,

			1, 2, 2, 4, 4, 3, 3, 1,

			5, 6, 6, 7, 7, 5
		};

		constexpr F32 CircleXYRadius = 0.5f;

		const static std::vector<SPositionVertex> CircleXY8 = 
		{
			// X      Y      Z      W
			{ 0.500000, 0.000000, 0.000000, 1.000000},
			{ 0.353694, 0.353413, 0.000000, 1.000000},
			{ 0.000398, 0.500000, 0.000000, 1.000000},
			{ -0.353131, 0.353975, 0.000000, 1.000000},
			{ -0.499999, 0.000796, 0.000000, 1.000000},
			{ -0.354256, -0.352849, 0.000000, 1.000000},
			{ -0.001194, -0.499999, 0.000000, 1.000000},
			{ 0.352567, -0.354537, 0.000000, 1.000000}
		};

		const static std::vector<U32> CircleXY8Indices =
		{
			0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 0, 
		};

		const static std::vector<SPositionVertex> CircleXY16 = 
		{
			// X      Y      Z      W
			{ 0.500000, 0.000000, 0.000000, 1.000000},
			{ 0.461978, 0.191250, 0.000000, 1.000000},
			{ 0.353694, 0.353413, 0.000000, 1.000000},
			{ 0.191618, 0.461825, 0.000000, 1.000000},
			{ 0.000398, 0.500000, 0.000000, 1.000000},
			{ -0.190882, 0.462130, 0.000000, 1.000000},
			{ -0.353131, 0.353975, 0.000000, 1.000000},
			{ -0.461673, 0.191985, 0.000000, 1.000000},
			{ -0.499999, 0.000796, 0.000000, 1.000000},
			{ -0.462282, -0.190514, 0.000000, 1.000000},
			{ -0.354256, -0.352849, 0.000000, 1.000000},
			{ -0.192353, -0.461520, 0.000000, 1.000000},
			{ -0.001194, -0.499999, 0.000000, 1.000000},
			{ 0.190146, -0.462433, 0.000000, 1.000000},
			{ 0.352567, -0.354537, 0.000000, 1.000000},
			{ 0.461366, -0.192720, 0.000000, 1.000000}
		};

		const static std::vector<U32> CircleXY16Indices =
		{
			0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 0, 
		};

		const static std::vector<SPositionVertex> CircleXY32 = 
		{
			// X      Y      Z      W
			{ 0.500000, 0.000000, 0.000000, 1.000000},
			{ 0.490402, 0.097496, 0.000000, 1.000000},
			{ 0.461978, 0.191250, 0.000000, 1.000000},
			{ 0.415818, 0.277661, 0.000000, 1.000000},
			{ 0.353694, 0.353413, 0.000000, 1.000000},
			{ 0.277992, 0.415597, 0.000000, 1.000000},
			{ 0.191618, 0.461825, 0.000000, 1.000000},
			{ 0.097887, 0.490325, 0.000000, 1.000000},
			{ 0.000398, 0.500000, 0.000000, 1.000000},
			{ -0.097106, 0.490480, 0.000000, 1.000000},
			{ -0.190882, 0.462130, 0.000000, 1.000000},
			{ -0.277330, 0.416039, 0.000000, 1.000000},
			{ -0.353131, 0.353975, 0.000000, 1.000000},
			{ -0.415375, 0.278323, 0.000000, 1.000000},
			{ -0.461673, 0.191985, 0.000000, 1.000000},
			{ -0.490246, 0.098277, 0.000000, 1.000000},
			{ -0.499999, 0.000796, 0.000000, 1.000000},
			{ -0.490557, -0.096715, 0.000000, 1.000000},
			{ -0.462282, -0.190514, 0.000000, 1.000000},
			{ -0.416259, -0.276998, 0.000000, 1.000000},
			{ -0.354256, -0.352849, 0.000000, 1.000000},
			{ -0.278653, -0.415153, 0.000000, 1.000000},
			{ -0.192353, -0.461520, 0.000000, 1.000000},
			{ -0.098668, -0.490168, 0.000000, 1.000000},
			{ -0.001194, -0.499999, 0.000000, 1.000000},
			{ 0.096325, -0.490634, 0.000000, 1.000000},
			{ 0.190146, -0.462433, 0.000000, 1.000000},
			{ 0.276667, -0.416480, 0.000000, 1.000000},
			{ 0.352567, -0.354537, 0.000000, 1.000000},
			{ 0.414931, -0.278984, 0.000000, 1.000000},
			{ 0.461366, -0.192720, 0.000000, 1.000000},
			{ 0.490089, -0.099058, 0.000000, 1.000000}
		};

		const static std::vector<U32> CircleXY32Indices =
		{
			0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 0,  
		};

		// TEMP
		const static std::vector<SPositionVertex> GenerateGridVertices(U64 segments)
		{
			std::vector<SPositionVertex> verts;
			SPositionVertex v;
			v.y = 0.0f;
			v.w = 1.0f;
			const F32 start = static_cast<F32>(segments / 2);
			for (U64 i = 0; i < segments + 1; i++)
			{
				v.x = -start + static_cast<F32>(i);
				v.z = -start;
				verts.push_back(v);
				v.z = start;
				verts.push_back(v);

				v.x = -start;
				v.z = -start + static_cast<F32>(i);
				verts.push_back(v);
				v.x = start;
				verts.push_back(v);
			}
			return verts;
		}

		const static std::vector<U32> GenerateGridIndices(U64 segments)
		{	
			std::vector<U32> indices;
			for (U64 i = 0; i < (segments + 1) * 4; i++)
			{
				indices.push_back(static_cast<U32>(i));
			}
			return indices;
		}

		const static std::vector<SPositionVertex> GridVertices = GenerateGridVertices(10);
		const static std::vector<U32> GridIndices = GenerateGridIndices(10);
		// ! TEMP

	}
}
