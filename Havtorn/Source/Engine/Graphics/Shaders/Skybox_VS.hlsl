// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "Includes/DeferredShaderStructs.hlsli"
#include "Includes/PointLightShaderStructs.hlsli"

PointLightVertexToPixel main(PointLightVertexInput input)
{
    PointLightVertexToPixel output;
    output.myPosition = input.myPosition;
    output.myWorldPosition = output.myPosition;
    
    float4x4 toCameraSpaceAtOrigin = ToCameraSpace;
    toCameraSpaceAtOrigin._14_24_34 = 0.0f;
    output.myPosition = mul(toCameraSpaceAtOrigin, output.myPosition);
    output.myPosition = mul(ToProjectionSpace, output.myPosition);
    
    output.myPosition.z = output.myPosition.w; // Make sure that the depth after w divide will be 1.0
    output.myUV = output.myPosition.xyw;
    output.myUV.y *= -1.0f;
    
    return output;
}
