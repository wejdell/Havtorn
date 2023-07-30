// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "Includes/SpriteShaderStructs.hlsli"

[maxvertexcount(4)]
void main(
	point VertexToGeometry input[1],
	inout TriangleStream<GeometryToPixelScreenSpace> output
)
{
    const float2 offset[4] =
    {
        { -1.0f, 1.0f },
        { 1.0f, 1.0f },
        { -1.0f, -1.0f },
        { 1.0f, -1.0f }
    };

    VertexToGeometry inputVertex = input[0];
    
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
        GeometryToPixelScreenSpace vertex;
        vertex.Position = 0.0f;
        vertex.Position.w = 1.0f;
        vertex.Position.xy += offset[index] * inputVertex.Size;
        float2 intermediate = vertex.Position.xy;
        vertex.Position.x = intermediate.x * cos(inputVertex.Rotation) - intermediate.y * sin(inputVertex.Rotation);
        vertex.Position.y = intermediate.x * sin(inputVertex.Rotation) + intermediate.y * cos(inputVertex.Rotation);
        vertex.Position.xy *= aspectRatioDivider;
        vertex.Position.xy += inputVertex.Position * 2.0f - 1.0f;
        vertex.Color = inputVertex.Color;
        vertex.UV = uv[index];
        output.Append(vertex);
    }
}