// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/LineShaderStructs.hlsli"

LinePixelOutput main(LineGeometryToPixel input)
{
    LinePixelOutput returnValue;
	returnValue.Color = input.Color;
    return returnValue;
}
