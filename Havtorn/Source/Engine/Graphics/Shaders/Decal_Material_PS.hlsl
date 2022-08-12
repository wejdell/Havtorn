#include "Includes/DecalShaderStructs.hlsli"

float4 main(VertexToPixel input) : SV_TARGET3
{
    float3 clipSpace = input.ClipSpacePosition;
    clipSpace.y *= -1.0f;
    float2 screenSpaceUV = (clipSpace.xy / clipSpace.z) * 0.5f + 0.5f;
    
    float z = depthTexture.Sample(defaultSampler, screenSpaceUV).r;
    float x = screenSpaceUV.x * 2.0f - 1;
    float y = (1 - screenSpaceUV.y) * 2.0f - 1;
    float4 projectedPos = float4(x, y, z, 1.0f);
    float4 viewSpacePos = mul(ToCameraFromProjection, projectedPos);
    viewSpacePos /= viewSpacePos.w;
    float4 worldPosFromDepth = mul(ToWorldFromCamera, viewSpacePos);
    
    float4 objectPosition = mul(toObjectSpace, worldPosFromDepth);
    clip(0.5f - abs(objectPosition.xyz));
    float2 decalUV = objectPosition.xy + 0.5f;
    decalUV.y *= -1.0f;

    float alpha = colorTexture.Sample(defaultSampler, decalUV).a;
    
    float3 material = materialTexture.Sample(defaultSampler, decalUV).rgb;
    float metalness = material.r;
    float perceptualRoughness = material.g;
    float emissive = material.b;
    float ambientOcclusion = normalTexture.Sample(defaultSampler, decalUV).b;
	
    return float4(metalness, perceptualRoughness, emissive, alpha/*ambientOcclusion*/);
}