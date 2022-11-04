// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "DeferredShaderStructs.hlsli"
#include "DetailNormalHelpers.hlsli"

float4 PixelShader_WorldPosition(float2 uv)
{       
    // Depth sampling
    float z = depthTexture.Sample(defaultSampler, uv).r;
    float x = uv.x * 2.0f - 1;
    float y = (1 - uv.y) * 2.0f - 1;
    const float4 projectedPos = float4(x, y, z, 1.0f);
    float4 viewSpacePos = mul(ToCameraFromProjection, projectedPos);
    viewSpacePos /= viewSpacePos.w;
    float4 worldPos = mul(ToWorldFromCamera, viewSpacePos);

    worldPos.a = 1.0f;
    return worldPos;
}

float PixelShader_DepthFromWorldPosition(float4 worldPos)
{
    float4 viewSpacePos = mul(ToCameraSpace, worldPos);
    float4 projectedPos = mul(ToProjectionSpace, viewSpacePos);

    projectedPos /= projectedPos.w;
    
    float2 uvCoords = projectedPos.xy;
    uvCoords *= float2(0.5f, -0.5f);
    uvCoords += float2(0.5f, 0.5f);
    
    return depthTexture.Sample(defaultSampler, uvCoords).r;
}

float4 PixelShader_Exists(float2 uv)
{
    //PixelOutput output;
    //float4 worldPos = PixelShader_WorldPosition(input).myColor.rgba;
    //output.myColor.rgba = worldPos;
    //return output;
    //PixelOutput output;
    float depth = depthTexture.Sample(defaultSampler, uv).r;

    return float4(depth, 0.0f, 0.0f, 0.0f);
}

float4 PixelShader_Albedo(float2 uv)
{
    return albedoTexture.Sample(defaultSampler, uv).rgba;
}

float4 PixelShader_Albedo(Texture2D anAlbedoTexture, float2 uv)
{
    float4 color = anAlbedoTexture.Sample(defaultSampler, uv).rgba;
    return color;
}

// This function is used for renderpasses to isolate the normal texture.
float4 PixelShader_NormalForIsolatedRendering(float2 uv)
{
    float3 normal;
    normal.xy = normalTexture.Sample(defaultSampler, uv).ag;
    // Recreate z
    normal.z = 0.0f;
    normal.z = sqrt(1 - saturate((normal.x * normal.x) + (normal.y * normal.y)));
    normal = normalize(normal);
    
    return float4(normal.x, normal.y, normal.z, 1.0f);
}

float4 PixelShader_Normal(float2 uv)
{
    float3 normal;
    normal.xy = normalTexture.Sample(defaultSampler, uv).ag;
    // Recreate z
    normal.z = 0.0f;
    normal = (normal * 2.0f) - 1.0f; // Comment this for Normal shader render pass
    normal.z = sqrt(1 - saturate((normal.x * normal.x) + (normal.y * normal.y)));
    normal = normalize(normal);
    
    return float4(normal.x, normal.y, normal.z, 1.0f);
}

void RecreateZ(inout float3 normal)
{
    normal.z = 0.0f;
    normal = (normal * 2.0f) - 1.0f;
    normal.z = sqrt(1 - saturate((normal.x * normal.x) + (normal.y * normal.y)));
    normal = normalize(normal);
}

float4 PixelShader_Normal(Texture2D aNormalTexture, float2 uv)
{
    float3 normal;
    normal.xy = aNormalTexture.Sample(defaultSampler, uv).ag;
    // Recreate z
    normal.z = 0.0f;
    normal = (normal * 2.0f) - 1.0f; // Comment this for Normal shader render pass
    normal.z = sqrt(1 - saturate((normal.x * normal.x) + (normal.y * normal.y)));
    //normal = (normal * 0.5f) + 0.5f;// Found in TGA modelviewer shader code, but seems to cause issues here.
    normal = normalize(normal);
    
    return float4(normal.x, normal.y, normal.z, 1.0f);
}

float PixelShader_DetailNormalStrength(float2 uv)
{
	const float output = materialTexture.Sample(defaultSampler, uv).a;
    return output;
}
float4 PixelShader_DetailNormal(float2 uv, int index)
{
	const float tilingModifier = DETAILNORMAL_TILING; // eq to scale
   
    float3 normal;
    
    normal.xy = detailNormals[index].Sample(defaultSampler, uv * tilingModifier).ag;
    normal.z = 0.0f;
    normal = (normal * 2.0f) - 1.0f;
    normal.z = sqrt(1 - saturate((normal.x * normal.x) + (normal.y * normal.y)));
    normal = normalize(normal);
    
    return float4(normal.x, normal.y, normal.z, 1.0f);
}

float4 PixelShader_Material(float2 uv)
{
    return materialTexture.Sample(defaultSampler, uv).rgba;
}

float4 PixelShader_Material(Texture2D aMaterialTexture, float2 uv)
{
    float4 material = aMaterialTexture.Sample(defaultSampler, uv).rgba;
    return material;
}

float PixelShader_AmbientOcclusion(float2 uv)
{
	return normalTexture.Sample(defaultSampler, uv).b;
}

float PixelShader_AmbientOcclusion(Texture2D aNormalTexture, float2 uv)
{
	const float ao = aNormalTexture.Sample(defaultSampler, uv).b;
    return ao;
}

float PixelShader_Metalness(float2 uv)
{
	return PixelShader_Material(uv).r;
}

float PixelShader_Metalness(Texture2D aMaterialTexture, float2 uv)
{
	const float metalness = PixelShader_Material(aMaterialTexture, uv).r;
    return metalness;
}

float PixelShader_PerceptualRoughness(float2 uv)
{
	const float roughness = PixelShader_Material(uv).g;
    return roughness;
}

float PixelShader_PerceptualRoughness(Texture2D aMaterialTexture, float2 uv)
{
	return PixelShader_Material(aMaterialTexture, uv).g;
}

float PixelShader_Emissive(float2 uv)
{
	const float emissive = PixelShader_Material(uv).b;
    return emissive;
}

float PixelShader_Emissive(Texture2D aMaterialTexture, float2 uv)
{
	return PixelShader_Material(aMaterialTexture, uv).b;
}

float PixelShader_SSAO(float2 uv)
{
	const float ssao = SSAOTexture.Sample(defaultSampler, uv).r;
    return ssao;
}

float SampleMaterialTexture(const int textureIndex, const int textureChannelIndex, const float2 uv)
{
    // NR: Texture2D materialChannelTextures[11] is not actually an array of Texture2Ds. In DirectX11,
    // this is just provided as syntactic sugar, and actually defines a bunch of textures in a row without
    // a wrapping struct. Because of this, it needs to resolve at compile time which texture to actually
    // sample from, and so the argument to the "array" must be a literal expression. Hence the switch below.
    
    float returnValue = 0.0f;
    
    switch (textureIndex)
    {
        case 0:
            returnValue = materialChannelTextures[MATERIAL_CHANNEL_0].Sample(defaultSampler, uv)[textureChannelIndex];
            break;
        case 1:
            returnValue = materialChannelTextures[MATERIAL_CHANNEL_1].Sample(defaultSampler, uv)[textureChannelIndex];
            break;
        case 2:
            returnValue = materialChannelTextures[MATERIAL_CHANNEL_2].Sample(defaultSampler, uv)[textureChannelIndex];
            break;
        case 3:
            returnValue = materialChannelTextures[MATERIAL_CHANNEL_3].Sample(defaultSampler, uv)[textureChannelIndex];
            break;
        case 4:
            returnValue = materialChannelTextures[MATERIAL_CHANNEL_4].Sample(defaultSampler, uv)[textureChannelIndex];
            break;
        case 5:
            returnValue = materialChannelTextures[MATERIAL_CHANNEL_5].Sample(defaultSampler, uv)[textureChannelIndex];
            break;
        case 6:
            returnValue = materialChannelTextures[MATERIAL_CHANNEL_6].Sample(defaultSampler, uv)[textureChannelIndex];
            break;
        case 7:
            returnValue = materialChannelTextures[MATERIAL_CHANNEL_7].Sample(defaultSampler, uv)[textureChannelIndex];
            break;
        case 8:
            returnValue = materialChannelTextures[MATERIAL_CHANNEL_8].Sample(defaultSampler, uv)[textureChannelIndex];
            break;
        case 9:
            returnValue = materialChannelTextures[MATERIAL_CHANNEL_9].Sample(defaultSampler, uv)[textureChannelIndex];
            break;
        case 10:
            returnValue = materialChannelTextures[MATERIAL_CHANNEL_10].Sample(defaultSampler, uv)[textureChannelIndex];
            break;
    }
    
    return returnValue;
}

float DetermineProperty(GPUMaterialProperty property, float2 uv)
{
    const float textureExists = property.TextureChannelIndex >= 0.0f;
    const float constantValueExists = property.ConstantValue >= 0.0f;
    
    return SampleMaterialTexture(property.TextureIndex, property.TextureChannelIndex, uv) * textureExists
            + property.ConstantValue * saturate(constantValueExists - textureExists);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
// GBUFFER  GBUFFER GBUFFER GBUFFER GBUFFER GBUFFER GBUFFER GBUFFER GBUFFER GBUFFER GBUFFER
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

float4 GBuffer_Albedo(float2 uv)
{
    float3 albedo = albedoTextureGBuffer.Sample(defaultSampler, uv).rgb;
    return float4(albedo.r, albedo.g, albedo.b, 1.0f);
}

float4 GBuffer_Normal(float2 uv)
{
    float3 normal = normalTextureGBuffer.Sample(defaultSampler, uv).rgb;

// Recreate z
    // Fr�ga bj�rn 18/2 :)
    //float3 normal;
    //normal.xy = normalTextureGBuffer.Sample(defaultSampler, input.myUV.xy).wy;
    //normal.z = 0.0f;
    //normal.z = sqrt(1.f - saturate((normal.x * normal.x) + (normal.y * normal.y)));   
    //normal.z = sqrt(1.f - (normal.x * normal.x) - (normal.y * normal.y));// Unity
    // random tests to see what works
        //normal = normalize(normal);
        //normal.z *= -1.0f;
        //normal = normal * 0.5 + 0.5;// dont do this
    
    return float4(normal.x, normal.y, normal.z, 1.0f);
}

float4 GBuffer_VertexNormal(float2 uv)
{
    float3 vertexNormal = vertexNormalTextureGBuffer.Sample(defaultSampler, uv).rgb;
    return float4(vertexNormal.r, vertexNormal.g, vertexNormal.b, 1.0f);
}

float GBuffer_AmbientOcclusion(float2 uv)
{
	const float ao = materialTextureGBuffer.Sample(defaultSampler, uv).a;
    return ao;
}

float GBuffer_Metalness(float2 uv)
{
	const float metalness = materialTextureGBuffer.Sample(defaultSampler, uv).r;
    return metalness;
}

float GBuffer_PerceptualRoughness(float2 uv)
{
	const float roughness = materialTextureGBuffer.Sample(defaultSampler, uv).g;
    return roughness;
}

float GBuffer_Emissive(float2 uv)
{
	const float emissive = materialTextureGBuffer.Sample(defaultSampler, uv).b;
    return emissive * EmissiveStrength;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
// SHADOWS SHADOWS SHADOWS SHADOWS SHADOWS SHADOWS SHADOWS SHADOWS SHADOWS SHADOWS SHADOWS
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

