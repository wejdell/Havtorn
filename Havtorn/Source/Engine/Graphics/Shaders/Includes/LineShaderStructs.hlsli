// Copyright 2022 Team Havtorn. All Rights Reserved.

struct LineVertexInput
{
    float4 Position   : POSITION;
    //float4 Color      : COLOR;
};
struct LineVertexToPixel
{
    float4 Position   : SV_POSITION;
    float4 Color      : COLOR;
};
struct LinePixelOutput
{
    float4 Color : SV_TARGET;
};