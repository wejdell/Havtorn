// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "Includes/SpriteShaderStructs.hlsli"

InstancedEditorVertexToGeometry main(InstancedEditorVertexInput input)
{
    InstancedEditorVertexToGeometry output;
    output.Transform = input.Transform;
    output.Color = input.Color;
    output.UVRect = input.UVRect;
    output.Entity = input.Entity;
	return output;
}