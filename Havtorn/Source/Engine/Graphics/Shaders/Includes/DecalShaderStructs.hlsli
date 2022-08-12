struct VertexInput
{
    float3 Position   : POSITION;
    float3 Normal     : NORMAL;
    float3 Tangent    : TANGENT;
    float3 Bitangent  : BINORMAL;
    float2 UV         : UV;
};

struct VertexToPixel
{
    float4 Position           : SV_POSITION;
    float3 ClipSpacePosition  : CLIP_SPACE_POSITION;
    float4 Normal             : NORMAL;
    float4 Tangent            : TANGENT;
    float4 Bitangent          : BITANGENT;
    float2 UV                 : UV;
};

Texture2D gBufferAlbedoTexture          : register(t1);
Texture2D gBufferNormalTexture          : register(t2);
Texture2D gBufferVertexNormalTexture    : register(t3);
Texture2D gBufferMaterialTexture        : register(t4);
Texture2D colorTexture                  : register(t5);
Texture2D materialTexture               : register(t6);
Texture2D normalTexture                 : register(t7);
Texture2D depthTexture                  : register(t21);
SamplerState defaultSampler             : register(s0);

cbuffer FrameBuffer : register(b0)
{
    float4x4 ToCameraSpace;
    float4x4 ToWorldFromCamera;
    float4x4 ToProjectionSpace;
    float4x4 ToCameraFromProjection;
    float4 CameraPosition;
};

cbuffer ObjectBuffer : register(b1)
{
    float4x4 toWorld;
    float4x4 toObjectSpace;
};