// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "Includes/SpriteShaderStructs.hlsli"

InstancedVertexToGeometry main(InstancedVertexInput input)
{
    InstancedVertexToGeometry output;
    output.Transform = input.Transform;
    output.Color = input.Color;
    output.UVRect = input.UVRect;
	return output;
}