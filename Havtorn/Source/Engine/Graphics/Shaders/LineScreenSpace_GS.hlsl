// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "Includes/LineShaderStructs.hlsli"

[maxvertexcount(27)]
void main(line LineVertexToGeometry input[2], inout TriangleStream<LineGeometryToPixel> outputStream)
{
    LineGeometryToPixel output;
    output.Color = input[0].Color;
    
    const float aspectRatio = (16.0f / 9.0f);
    const float4 horizontal = float4(1, 0, 0, 0) / aspectRatio;
    const float4 vertical = float4(0, 1, 0, 0);
    
    const float4 hIncrement = horizontal * HalfThickness;
    const float4 vIncrement = vertical * HalfThickness;
    
    /*
           .
        .  o  .     o = inputPos, . = vert
           .
    
        t = top, l = left, r = right, b = bottom
        
        t = inputPos + vIncrement
        r = inputPos + hIncrement
        b = inputPos - vIncrement
        l = inputPos - hIncrement
    */

    //float4 vertexObjectPos = input[0].Position.xyzw / input[0].Position.w;
    float4 vertexObjectPos = float4(0, 0, 0, 1);
    float4 vertexWorldPos = mul(ToWorld, vertexObjectPos);
    
    const float4 vertexPos1Top = vertexWorldPos + vIncrement;
    const float4 vertexPos1Right = vertexWorldPos + hIncrement;
    const float4 vertexPos1Bot = vertexWorldPos - vIncrement;
    const float4 vertexPos1Left = vertexWorldPos - hIncrement;

    //float4 vertexWorldPos2 = input[1].Position.xyzw / input[1].Position.w;
    float4 vertexObjectPos2 = float4(0, 0, 1, 1);
    float4 vertexWorldPos2 = mul(ToWorld, vertexObjectPos2);
    
    const float4 vertexPos2Top = vertexWorldPos2 + vIncrement;
    const float4 vertexPos2Right = vertexWorldPos2 + hIncrement;
    const float4 vertexPos2Bot = vertexWorldPos2 - vIncrement;
    const float4 vertexPos2Left = vertexWorldPos2 - hIncrement;
        
    // Plane on Point 1 verts:[5]
    output.Position = vertexPos1Left;
    outputStream.Append(output);
    output.Position = vertexPos1Top;
    outputStream.Append(output);
    output.Position = vertexPos1Right;
    outputStream.Append(output);
    output.Position = vertexPos1Bot;
    outputStream.Append(output);
    output.Position = vertexPos1Left;
    outputStream.Append(output);
    
    // Tri1 of Plane: Left - Right verts:[4]
    output.Position = vertexPos1Left;
    outputStream.Append(output);
    output.Position = vertexPos2Left;
    outputStream.Append(output);
    output.Position = vertexPos2Right;
    outputStream.Append(output);
    output.Position = vertexPos1Left;
    outputStream.Append(output);
    
    // Tri2 of Plane: verts:[4]
    output.Position = vertexPos1Left;
    outputStream.Append(output);
    output.Position = vertexPos1Right;
    outputStream.Append(output);
    output.Position = vertexPos2Right;
    outputStream.Append(output);
    output.Position = vertexPos1Left;
    outputStream.Append(output);
    
    // verts:[1]
    output.Position = vertexWorldPos;
    outputStream.Append(output);
    
    // Tri1 of Plane: Top - Bot verts:[4]
    output.Position = vertexPos1Top;
    outputStream.Append(output);
    output.Position = vertexPos2Top;
    outputStream.Append(output);
    output.Position = vertexPos2Bot;
    outputStream.Append(output);
    output.Position = vertexPos1Top;
    outputStream.Append(output);
    
    // Tri2 of Plane: Top - Bot verts:[4]
    output.Position = vertexPos1Top;
    outputStream.Append(output);
    output.Position = vertexPos1Bot;
    outputStream.Append(output);
    output.Position = vertexPos2Bot;
    outputStream.Append(output);
    output.Position = vertexPos1Top;
    outputStream.Append(output);
 
    // Plane on Point 2 verts:[5]
    output.Position = vertexPos2Top;
    outputStream.Append(output);
    output.Position = vertexPos2Right;
    outputStream.Append(output);
    output.Position = vertexPos2Bot;
    outputStream.Append(output);
    output.Position = vertexPos2Left;
    outputStream.Append(output);
    output.Position = vertexPos2Top;
    outputStream.Append(output);
    
    outputStream.RestartStrip();
}

