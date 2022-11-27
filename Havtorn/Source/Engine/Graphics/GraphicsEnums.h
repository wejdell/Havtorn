// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn 
{
	enum class EShaderType
	{
		Vertex,
		Compute,
		Geometry,
		Pixel
	};

	enum class ESamplerType
	{
		Border,
		Clamp,
		Mirror,
		Wrap
	};

	enum class EInputLayoutType
	{
		Pos3Nor3Tan3Bit3UV2,
		Pos3Nor3Tan3Bit3UV2Trans,
		Pos4
	};

	enum class EVertexShaders
	{
		Fullscreen = 0,
		StaticMesh = 1,
		StaticMeshInstanced = 2,
		Decal = 3,
		PointAndSpotLight = 4,
		EditorPreview = 5,
		Line = 6,
	};

	enum class EPixelShaders
	{
		GBuffer = 0,
		DecalAlbedo = 1,
		DecalMaterial = 2,
		DecalNormal = 3,
		DeferredDirectional = 4,
		DeferredPoint = 5,
		DeferredSpot = 6,
		VolumetricDirectional = 7,
		VolumetricPoint = 8,
		VolumetricSpot = 9,
		EditorPreview = 10,
		Line = 11,
	};

	enum class EGeometryShaders
	{
		Line = 0,
	};

	enum class ESamplers
	{
		DefaultWrap = 0,
		DefaultBorder = 1,
	};

	enum class ETopologies
	{
		TriangleList = 0,
		LineList = 1,
	};

	// Should be 1:1 to RenderManager::InitVertexBufferPrimitives()
	// Geometries used can be found in GeometryPrimitives.h
	// GeometryPrimitivesUtility.h has a std::map that simplifies access to the primitives the enum refers to.
	// EVertexBufferPrimitives Count should not exceed U8 max (255)
	enum class EVertexBufferPrimitives
	{
		DecalProjector,
		PointLightCube,
		Icosphere,
		LineShape,
		Arrow,
		DebugCube,
		Camera,
		CircleXY8,
		CircleXY16,
		CircleXY32,
		Grid,
		//TODO.ANYONE: add more debug shape primitives.
	};

	// GeometryPrimitivesUtility.h has a std::map that simplifies access to the primitives the enum refers to.
	// EDefaultIndexBuffers Count should not exceed U8 max (255)
	enum class EDefaultIndexBuffers
	{
		DecalProjector,
		PointLightCube,
		LineShape,
		Arrow,
		DebugCube,
		Camera,
		CircleXY8,
		CircleXY16,
		CircleXY32,
		Grid,
		//TODO.ANYONE: add more debug shape primitives.
	};

	enum class EIndexBufferPrimitives
	{
		DecalProjector,
		PointLightCube,
		Icosphere
	};

	enum class ETextureFormat
	{
		DDS,
		TGA
	};

	enum class EMaterialConfiguration
	{
		AlbedoMaterialNormal_Packed
	};

	enum class EMaterialProperty
	{
		AlbedoR,
		AlbedoG,
		AlbedoB,
		AlbedoA,
		NormalX,
		NormalY,
		NormalZ,
		AmbientOcclusion,
		Metalness,
		Roughness,
		Emissive,
		Count
	};
}