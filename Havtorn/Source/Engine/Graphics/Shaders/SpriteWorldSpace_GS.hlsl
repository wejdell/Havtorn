// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "Includes/SpriteShaderStructs.hlsli"

[maxvertexcount(4)]
void main(point InstancedVertexToGeometry input[1], inout TriangleStream<GeometryToPixelWorldSpace> output)
{
    const float2 offset[4] =
    {
        { -1.0f, 1.0f },
        { 1.0f, 1.0f },
        { -1.0f, -1.0f },
        { 1.0f, -1.0f }
    };

    InstancedVertexToGeometry inputVertex = input[0];
    
    const float2 uv[4] =
    {
        { inputVertex.UVRect.x, inputVertex.UVRect.y },
        { inputVertex.UVRect.z, inputVertex.UVRect.y },
        { inputVertex.UVRect.x, inputVertex.UVRect.w },
        { inputVertex.UVRect.z, inputVertex.UVRect.w }
    };
    
    float2 aspectRatioDivider = { 9.0f / 16.0f, 1.0f };
    for (unsigned int index = 0; index < 4; index++)
    {
        GeometryToPixelWorldSpace vertex;
        
        const float4 vertexObjectPos = float4(offset[index].x, offset[index].y, 0.0f, 1.0f);
        const float4 vertexWorldPos = mul(inputVertex.Transform, vertexObjectPos);
        const float4 vertexViewPos = mul(ToCameraSpace, vertexWorldPos);
        const float4 vertexProjectionPos = mul(ToProjectionSpace, vertexViewPos);

        const float3x3 toWorldRotation = (float3x3) inputVertex.Transform;
        float3 vertexWorldNormal = mul(toWorldRotation, float3(0.0f, 0.0f, -1.0f));
        float3 vertexWorldTangent = mul(toWorldRotation, float3(0.0f, 1.0f, 0.0f));
        float3 vertexWorldBinormal = mul(toWorldRotation, float3(1.0f, 0.0f, 0.0f));

        vertex.Position = vertexProjectionPos;
        vertex.Normal = float4(vertexWorldNormal, 0);
        vertex.Tangent = float4(vertexWorldTangent, 0);
        vertex.Bitangent = float4(vertexWorldBinormal, 0);
        vertex.Color = inputVertex.Color;
        vertex.UV = uv[index];
        output.Append(vertex);
    }
}