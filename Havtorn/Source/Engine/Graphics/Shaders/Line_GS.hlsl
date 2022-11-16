// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/LineShaderStructs.hlsli"

[maxvertexcount(27)]
void main(line LineVertexToPixel input[2], inout TriangleStream<LineVertexToPixel> outputStream)
{
    LineVertexToPixel output;
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

    const float4 vertexPos1Top   = input[0].Position + vIncrement;
    const float4 vertexPos1Right = input[0].Position + hIncrement;
    const float4 vertexPos1Bot   = input[0].Position - vIncrement;
    const float4 vertexPos1Left  = input[0].Position - hIncrement;
    
    const float4 vertexPos2Top   = input[1].Position + vIncrement;
    const float4 vertexPos2Right = input[1].Position + hIncrement;
    const float4 vertexPos2Bot   = input[1].Position - vIncrement;
    const float4 vertexPos2Left  = input[1].Position - hIncrement;
        
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
    output.Position = input[0].Position;
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

