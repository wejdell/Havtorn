// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/FullscreenShaderStructs.hlsli"
#include "Includes/VolumetricLightShaderStructs.hlsli"
#include "Includes/MathHelpers.hlsli"
#include "Includes/ShadowSampling.hlsli"

cbuffer LightBuffer : register(b1)
{
    float4 toDirectionalLight;
    float4 directionalLightColor;
}

cbuffer ShadowmapBuffer : register(b5)
{
    struct SShadowmapViewData
    {
        float4x4 ToShadowMapView;
        float4x4 ToShadowMapProjection;
        float4 ShadowmapPosition;
        float2 ShadowmapResolution;
        float2 ShadowAtlasResolution;
        float2 ShadowmapStartingUV;
        float ShadowTestTolerance;
        float Padding;
    } ShadowmapViewData[6];
}

float4 PixelShader_WorldPosition(float2 uv)
{
    // Depth sampling
    float z = depthTexture.Sample(defaultSampler, uv).r;
    float x = uv.x * 2.0f - 1;
    float y = (1 - uv.y) * 2.0f - 1;
    float4 projectedPos = float4(x, y, z, 1.0f);
    float4 viewSpacePos = mul(toCameraFromProjection, projectedPos);
    viewSpacePos /= viewSpacePos.w;
    float4 worldPos = mul(toWorldFromCamera, viewSpacePos);

    worldPos.a = 1.0f;
    return worldPos;
}

void ExecuteRaymarching(inout float3 rayPositionLightVS, float3 invViewDirLightVS, float stepSize, float l, inout float3 VLI, SShadowmapViewData viewData)
{
    rayPositionLightVS.xyz += stepSize * invViewDirLightVS.xyz;
    
    float3 visibilityTerm = ShadowFactor(rayPositionLightVS.xyz, viewData.ToShadowMapProjection, shadowDepthTexture, shadowSampler, viewData.ShadowmapResolution, viewData.ShadowAtlasResolution, viewData.ShadowmapStartingUV, viewData.ShadowTestTolerance).xxx;
    //float3 visibilityTerm = 1.0f;
    
    // Distance to the current position on the ray in light view-space
    float d = length(rayPositionLightVS.xyz) /*20.0f*/;
    float dRcp = rcp(d); // reciprocal
    
    // Calculate the final light contribution for the sample on the ray...
    float phase = 0.25f * PI_RCP;
    //float projection = dot(invViewDirLightVS, -toDirectionalLight.xyz);
    //phase = PhaseFunctionHenyeyGreenstein(projection, henyeyGreensteinGValue);
    float3 intens = scatteringProbability * (visibilityTerm * (lightPower * phase) * dRcp * dRcp) * exp(-d * scatteringProbability) * exp(-l * scatteringProbability) * stepSize;
    
    // ... and add it to the total contribution of the ray
    VLI += intens;
}

// !RAYMARCHING

PixelOutput main(VertexToPixel input)
{
    PixelOutput output;
    
    float raymarchDistanceLimit = 99999.0f;
    
    float4 worldPosition = PixelShader_WorldPosition(input.myUV);
    float4 camPos = cameraPosition;
    
    SShadowmapViewData viewData = ShadowmapViewData[0];
    
    worldPosition.xyz -= viewData.ShadowmapPosition.xyz;
    float4 positionLightVS = mul(viewData.ToShadowMapView, worldPosition);
   
    camPos.xyz -= viewData.ShadowmapPosition.xyz;
    float4 cameraPositionLightVS = mul(viewData.ToShadowMapView, camPos);
    
    // Reduce noisyness by truncating the starting position
    //float raymarchDistance = trunc(clamp(length(cameraPositionLightVS.xyz - positionLightVS.xyz), 0.0f, raymarchDistanceLimit));
    float4 invViewDirLightVS = float4(normalize(cameraPositionLightVS.xyz - positionLightVS.xyz), 0.0f);
    float raymarchDistance = /*trunc(*/clamp(length(cameraPositionLightVS.xyz - positionLightVS.xyz), 0.0f, raymarchDistanceLimit) /*)*/;
    
    // Calculate the size of each step
    float stepSize = raymarchDistance * numberOfSamplesReciprocal;
    
    // Calculate the offsets on the ray according to the interleaved sampling pattern
    float2 interleavedPos = fmod(input.myPosition.xy - 0.5f, INTERLEAVED_GRID_SIZE);
#if defined(USE_RANDOM_RAY_SAMPLES)
    float index = (floor(interleavedPos.y) * INTERLEAVED_GRID_SIZE + floor(interleavedPos.x));
    // lightVolumetricRandomRaySamples contains the values 0..63 in a randomized order
    float rayStartOffset = lightVolumetricRandomRaySamples[index] * (stepSize * INTERLEAVED_GRID_SIZE_SQR_RCP);
#else
    float rayStartOffset = (interleavedPos.y * INTERLEAVED_GRID_SIZE + interleavedPos.x) * (stepSize * INTERLEAVED_GRID_SIZE_SQR_RCP);
#endif // USE_RANDOM_RAY_SAMPLES
    
    float3 rayPositionLightVS = rayStartOffset * invViewDirLightVS.xyz + positionLightVS.xyz;
    
    // The total light contribution accumulated along the ray
    float3 VLI = 0.0f;
    
    // Start ray marching
    [loop]
    for (float l = raymarchDistance; l > 2.0f * stepSize; l -= stepSize)
    {
        ExecuteRaymarching(rayPositionLightVS, invViewDirLightVS.xyz, stepSize, l, VLI, viewData);
    }
    
    //float fogDensity = scatteringProbability;
    
    //float depth = depthTexture.Sample(defaultSampler, input.myUV).r;
    //depth = (depth - 0.95f) / 0.05f;
    //float3 VLI = exp(depth/* * fogDensity*/);
    
    output.myColor.rgb = directionalLightColor.rgb * VLI;
    output.myColor.a = 1.0f;
    return output;
}