// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/LineShaderStructs.hlsli"

[maxvertexcount(16)]
void main(line LineVertexToPixel input[2], inout TriangleStream<LineVertexToPixel> outputStream)
{
    LineVertexToPixel output;
    output.Color = input[0].Color;

    // Get direction from input[0].pos -> input[1].pos
    // Get normal of direction
    // output.pos1 = input[0].pos + normalOfDirection.normalized * thickness
    // output.pos2 = input[0].pos - normalOfDirection.normalized * thickness
    // we have the object's transform float4x4, r1 == horizontal, r2 == vertical?, r3 == frwdir?
    
    //float4 directionOfLine = input[1].Position - input[0].Position;
    //directionOfLine = normalize(directionOfLine);
    
    float4 horizontal = /*ToWorld._11_12_13_14*/float4(1, 0, 0, 0);
    float4 vertical = /*ToWorld._21_22_23_24*/float4(0, 1, 0, 0);
    
    float thickness = 0.005f;
    float4 hThicknessIncrement = normalize(horizontal) * thickness;
    float4 vThicknessIncrement = normalize(vertical) * thickness;
    
    //float4 vertexObjectPos = input.Position.xyzw;
    //float4 vertexWorldPos = mul(ToWorld, vertexObjectPos);
    //float4 vertexViewPos = mul(ToCameraSpace, vertexWorldPos);
    //float4 vertexProjectionPos = mul(ToProjectionSpace, vertexViewPos);
    
    // H
    // h tri1
    output.Position = input[0].Position + hThicknessIncrement;
    outputStream.Append(output);
    
    output.Position = input[0].Position - hThicknessIncrement;
    outputStream.Append(output);
    
    output.Position = input[1].Position - hThicknessIncrement;
    outputStream.Append(output);
    
    output.Position = input[0].Position + hThicknessIncrement;
    outputStream.Append(output);
    
    // h tri2
    output.Position = input[0].Position + hThicknessIncrement;
    outputStream.Append(output);
    
    output.Position = input[1].Position - hThicknessIncrement;
    outputStream.Append(output);
    
    output.Position = input[1].Position + hThicknessIncrement;
    outputStream.Append(output);

    output.Position = input[0].Position + hThicknessIncrement;
    outputStream.Append(output);
    
    // V
    // v tri1
    output.Position = input[0].Position + vThicknessIncrement;
    outputStream.Append(output);
    
    output.Position = input[0].Position - vThicknessIncrement;
    outputStream.Append(output);
    
    output.Position = input[1].Position - vThicknessIncrement;
    outputStream.Append(output);
    
    output.Position = input[0].Position + vThicknessIncrement;
    outputStream.Append(output);
    
    // v tri2
    output.Position = input[0].Position + vThicknessIncrement;
    outputStream.Append(output);
    
    output.Position = input[1].Position - vThicknessIncrement;
    outputStream.Append(output);
    
    output.Position = input[1].Position + vThicknessIncrement;
    outputStream.Append(output);

    output.Position = input[0].Position + vThicknessIncrement;
    outputStream.Append(output);
    
    //for (uint i = 0; i < 2; i++)
    //{
    //    output.Position = input[i].Position + hThicknessIncrement;
    //    outputStream.Append(output);
        
    //    output.Position = input[i].Position - hThicknessIncrement;
    //    outputStream.Append(output);
        
    //    output.Position = input[i].Position - hThicknessIncrement;
    //    outputStream.Append(output);
    //}
    
    outputStream.RestartStrip();
}