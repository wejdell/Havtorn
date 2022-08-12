// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/PointLightShaderStructs.hlsli"
#include "Includes/VolumetricLightShaderStructs.hlsli"
#include "Includes/MathHelpers.hlsli"
#include "Includes/ShadowSampling.hlsli"

sampler defaultSampler : register(s0);

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
    float4 viewSpacePos = mul(pointLightToCameraFromProjection, projectedPos);
    viewSpacePos /= viewSpacePos.w;
    float4 worldPos = mul(pointLightToWorldFromCamera, viewSpacePos);

    worldPos.a = 1.0f;
    return worldPos;
}

void ExecuteRaymarching(inout float3 rayPositionLightVS, float3 invViewDirLightVS, inout float3 rayPositionWorld, float3 invViewDirWorld, float stepSize, float l, inout float3 VLI/*, SShadowmapViewData viewData*/)
{
    rayPositionLightVS.xyz += stepSize * invViewDirLightVS.xyz;

    // March in world space in parallel
    rayPositionWorld += stepSize * invViewDirWorld;
    //..
    
    int shadowmapViewIndex = GetShadowmapViewIndex(rayPositionWorld.xyz, pointLightPositionAndRange.xyz);
    SShadowmapViewData viewData = ShadowmapViewData[shadowmapViewIndex];
    
    float3 visibilityTerm = 1.0f - ShadowFactor(rayPositionWorld, viewData.ShadowmapPosition.xyz, viewData.ToShadowMapView, viewData.ToShadowMapProjection, shadowDepthTexture, shadowSampler, viewData.ShadowmapResolution, viewData.ShadowAtlasResolution, viewData.ShadowmapStartingUV, viewData.ShadowTestTolerance).xxx;
    
    // Distance to the current position on the ray in light view-space
    float d = length(rayPositionLightVS.xyz);
    float dRcp = rcp(d); // reciprocal
    
    // Calculate the final light contribution for the sample on the ray...
    float3 intens = scatteringProbability * (visibilityTerm * (lightPower * 0.25 * PI_RCP) * dRcp * dRcp) * exp(-d * scatteringProbability) * exp(-l * scatteringProbability) * stepSize;
    
    // World space attenuation
    float3 toLight = pointLightPositionAndRange.xyz - rayPositionWorld.xyz;
    float lightDistance = length(toLight);
    toLight = normalize(toLight);
    float linearAttenuation = lightDistance / pointLightPositionAndRange.w;
    linearAttenuation = 1.0f - linearAttenuation;
    linearAttenuation = saturate(linearAttenuation);
    float physicalAttenuation = saturate(1.0f / (lightDistance * lightDistance));
    //float lambert = saturate(dot(normal, toLight));
    float attenuation = /*lambert **/linearAttenuation * physicalAttenuation;
    
    // ... and add it to the total contribution of the ray
    VLI += intens * attenuation;
}

// !RAYMARCHING

PointLightPixelOutput main(PointLightVertexToPixel input)
{
    PointLightPixelOutput output;
    
    float raymarchDistanceLimit = 2.0f * pointLightPositionAndRange.w;
     
    // Texture Depth
    float2 screenUV = (input.myUV.xy / input.myUV.z) * 0.5f + 0.5f;
    const float textureDepth = depthTexture.Sample(defaultSampler, screenUV).r;
   
    // World Pos Depth
    float4 viewSpacePos = mul(pointLightToCamera, input.myWorldPosition);
    float4 projectedPos = mul(pointLightToProjection, viewSpacePos);
    projectedPos /= projectedPos.w;
    const float worldPosDepth = projectedPos.z;
   
    // World pos from light geometry
    float4 worldPosition = input.myWorldPosition;
    
    if (textureDepth < worldPosDepth)
    {
        // World pos from texture depth
        float z = textureDepth;
        float x = screenUV.x * 2.0f - 1;
        float y = (1 - screenUV.y) * 2.0f - 1;
        const float4 projectedPos = float4(x, y, z, 1.0f);
        float4 viewSpacePos = mul(pointLightToCameraFromProjection, projectedPos);
        viewSpacePos /= viewSpacePos.w;
        worldPosition = mul(pointLightToWorldFromCamera, viewSpacePos);

        worldPosition.a = 1.0f;
    }
    
    float4 camPos = pointLightCameraPosition;
    
    int shadowmapViewIndex = GetShadowmapViewIndex(worldPosition.xyz, pointLightPositionAndRange.xyz);
    SShadowmapViewData viewData = ShadowmapViewData[shadowmapViewIndex];
    
    // For marching in world space in parallel
    float3 rayPositionWorld = worldPosition.xyz;
    float3 invViewDirWorld = normalize(camPos - worldPosition).xyz;
    
    worldPosition.xyz -= viewData.ShadowmapPosition.xyz;
    float4 positionLightVS = mul(viewData.ToShadowMapView, worldPosition);
   
    camPos.xyz -= viewData.ShadowmapPosition.xyz;
    float4 cameraPositionLightVS = mul(viewData.ToShadowMapView, camPos);
    
    // Reduce noisyness by truncating the starting position
    //float raymarchDistance = trunc(clamp(length(cameraPositionLightVS.xyz - positionLightVS.xyz), 0.0f, raymarchDistanceLimit));
    float4 invViewDirLightVS = float4(normalize(cameraPositionLightVS.xyz - positionLightVS.xyz), 0.0f);
    float raymarchDistance = clamp(length(cameraPositionLightVS.xyz - positionLightVS.xyz), 0.0f, raymarchDistanceLimit);
    
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
    for (float l = raymarchDistance; l > stepSize; l -= stepSize)
    {
        ExecuteRaymarching(rayPositionLightVS, invViewDirLightVS.xyz, rayPositionWorld, invViewDirWorld, stepSize, l, VLI/*, viewData*/);
    }
    
    output.myColor.rgb = pointLightColorAndIntensity.rgb * VLI;
    output.myColor.a = 1.0f;
    return output;
}