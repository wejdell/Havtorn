// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "Includes/SpriteShaderStructs.hlsli"

VertexToGeometry main(VertexInput input)
{
    VertexToGeometry output;
    output.Position = Position;
    output.Color = Color;
    output.UVRect = UVRect;
    output.Size = Size;
    output.Rotation = Rotation;
	return output;
}