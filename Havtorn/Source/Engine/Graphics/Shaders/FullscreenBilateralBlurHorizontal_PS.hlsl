// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/FullscreenShaderStructs.hlsli"

PixelOutput main(VertexToPixel input)
{
    PixelOutput returnValue;
	
    float texelSize = 1.0f / resolution.x;
    float3 blurColor = float3(0.0f, 0.0f, 0.0f);
    float normalizationFactor = 0.0f;
    const float bZ = 1.0 / normpdf(0.0, BSIGMA);
    const float3 originalPixelValue = fullscreenTexture1.Sample(defaultSampler, input.UV.xy).rgb;

    const unsigned int kernelSize = 5;
    const float start = (((float) (kernelSize) - 1.0f) / 2.0f) * -1.0f;
    for (unsigned int i = 0; i < kernelSize; i++)
    {
	    const float2 uv = input.UV.xy + float2(texelSize * (start + (float) i), 0.0f);
	    const float3 resource = fullscreenTexture1.Sample(defaultSampler, uv).rgb;
	    const float colorFactor = normpdf3(resource - originalPixelValue, BSIGMA) * bZ * gaussianKernel5[i];
        normalizationFactor += colorFactor;
        blurColor += resource * colorFactor;
    }
	
    returnValue.Color.rgb = blurColor / normalizationFactor;
    returnValue.Color.a = 1.0f;
    return returnValue; 
};