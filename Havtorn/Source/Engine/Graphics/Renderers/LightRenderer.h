// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "hvpch.h"
#include <array>

#include "Graphics/GraphicsEnums.h"
#include "Graphics/GraphicsStructs.h"
#include <RHI/RenderingPrimitives/DataBuffer.h>
#include <RHI/RenderingPrimitives/RenderTexture.h>

namespace Havtorn
{
	class CRHI;
	class CRenderManager;
	class CRenderStateManager;

	enum class EDepthPrePassGeometryType
	{
		StaticMesh,
		SkeletalMesh
	};

	enum class ELightType
	{
		Directional,
		Point,
		Spot,
		Area
	};

	class CLightRenderer 
	{
	public:
		friend CRenderManager;

	private:
		struct SDirectionalLightBufferData
		{
			SVector4 ToDirectionalLight;
			SVector4 DirectionalLightColor;
		} DirectionalLightBufferData;
		HV_ASSERT_BUFFER(SDirectionalLightBufferData)

		struct SPointLightBufferData
		{
			SMatrix ToWorldFromObject;
			SVector4 ColorAndIntensity;
			SVector4 PositionAndRange;
		} PointLightBufferData;
		HV_ASSERT_BUFFER(SPointLightBufferData)

		struct SSpotLightBufferData
		{
			SVector4 ColorAndIntensity;
			SVector4 PositionAndRange;
			SVector4 Direction;
			SVector4 DirectionNormal1;
			SVector4 DirectionNormal2;
			F32 OuterAngle = 0.0f;
			F32 InnerAngle = 0.0f;
			SVector2<F32> Padding;
		} SpotLightBufferData;
		HV_ASSERT_BUFFER(SSpotLightBufferData)

	private:
		CLightRenderer() = default;
		~CLightRenderer();
		bool Init(CRHI* rhi, CRenderManager* manager, CRenderStateManager* stateManager);
		U64 RenderStaticMeshDepthPrePass(const std::vector<SShadowmapViewData>& shadowmapData, const std::vector<SDrawCallData>& drawCallData, const std::vector<SMatrix>& instanceTransforms, const U64 currentPSOHash);

		CDataBuffer DirectionalLightBuffer;
		CDataBuffer PointLightBuffer;
		CDataBuffer SpotLightBuffer;
		
		SFrameBufferData FrameBufferData;
		CDataBuffer FrameBuffer;

		CDataBuffer InstancedTransformBuffer;

		U64 StaticMeshShadowPassPSOIndex = 0;
		U64 DirectonalLightPSOIndex = 0;
		U64 PointLightPSOIndex = 0;
		U64 SpotLightPSOIndex = 0;
		U64 VolumetricDirectionalLightPSOIndex = 0;
		U64 VolumetricPointLightPSOIndex = 0;
		U64 VolumetricSpotLightPSOIndex = 0;

		CRenderManager* Manager = nullptr;
		CRenderStateManager* RenderStateManager = nullptr;
	};
}
