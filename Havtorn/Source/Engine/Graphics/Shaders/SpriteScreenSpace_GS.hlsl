// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "Includes/SpriteShaderStructs.hlsli"

[maxvertexcount(4)]
void main(point InstancedVertexToGeometry input[1], inout TriangleStream<GeometryToPixelScreenSpace> output)
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
        const float3x3 localRotation = (float3x3) inputVertex.Transform;
        float2 screenSpacePosition = inputVertex.Transform._14_24;
        
        GeometryToPixelScreenSpace vertex;
        vertex.Position = 0.0f;
        vertex.Position.w = 1.0f;
        vertex.Position.xy += offset[index];
        vertex.Position.xyz = mul(localRotation, vertex.Position.xyz);
        vertex.Position.xy *= aspectRatioDivider;
        vertex.Position.xy += screenSpacePosition * 2.0f - 1.0f;
        vertex.Color = inputVertex.Color;
        vertex.UV = uv[index];
        output.Append(vertex);
    }
}