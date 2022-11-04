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
			{ -1.0f, -1.0f, -1.0f,  1.0f },
			{  1.0f, -1.0f, -1.0f,  1.0f },
			{ -1.0f,  1.0f, -1.0f,  1.0f },
			{  1.0f,  1.0f, -1.0f,  1.0f },
			{ -1.0f, -1.0f,  1.0f,  1.0f },
			{  1.0f, -1.0f,  1.0f,  1.0f },
			{ -1.0f,  1.0f,  1.0f,  1.0f },
			{  1.0f,  1.0f,  1.0f,  1.0f }
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

		static std::vector<SPositionVertex> Line =
		{
			{ 0.0f, 0.0f, 0.0f,  1.0f },
			{  0.0f, 0.0f, 1.0f,  1.0f }
		};

		// Without Indices
		static std::vector<SPositionVertex> FlatArrow =
		{
			{ 0.0f, 0.0f, 0.0f,  1.0f },
			{  0.0f, 0.0f, 1.0f,  1.0f },
			{  0.15f, 0.0f, 0.8f,  1.0f },
			{  0.0f, 0.0f, 1.0f,  1.0f },
			{  -0.15f, 0.0f, 0.8f,  1.0f },
			{  0.0f, 0.0f, 1.0f,  1.0f },
		}; 
	}
}
