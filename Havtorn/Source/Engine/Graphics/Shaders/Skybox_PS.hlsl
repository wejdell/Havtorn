// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "Includes/DeferredShaderStructs.hlsli"
#include "Includes/PointLightShaderStructs.hlsli"

float4 main(PointLightVertexToPixel input) : SV_TARGET
{
    return environmentTexture.Sample(defaultSampler, input.myWorldPosition.rgb);
}