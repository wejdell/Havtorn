// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/FullscreenShaderStructs.hlsli"

float3 Uncharted2Tonemap(const float3 x)
{
	const float a = 0.15f;
	const float b = 0.5f;
	const float c = 0.1f;
	const float d = 0.2f;
	const float e = 0.02f;
	const float f = 0.3f;
    
    return ((x * (a * x + c * b) + d * e) / (x * (a * x + b) + d * f)) - e / f;
}

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static const float3x3 ACESInputMat =
{
    { 0.59719, 0.35458, 0.04823 },
    { 0.07600, 0.90834, 0.01566 },
    { 0.02840, 0.13383, 0.83777 }
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367 },
    { -0.10208, 1.10813, -0.00605 },
    { -0.00327, -0.07276, 1.07602 }
};

float3 RRTAndODTFit(float3 v)
{
	const float3 a = v * (v + 0.0245786f) - 0.000090537f;
	const float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 ACESFitted(float3 color)
{
    color = mul(ACESInputMat, color);

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = mul(ACESOutputMat, color);

    // Clamp to [0, 1]
    color = saturate(color);

    return color;
}

float3 ACESApprox(const float3 x)
{
	const float a = 2.51f;
	const float b = 0.03f;
	const float c = 2.43f;
	const float d = 0.59f;
	const float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// AgX from: https://www.shadertoy.com/view/dtSGD1
static const float MIN_EV = -10.0f;
static const float MAX_EV = 6.5f;

float3x3 InverseMat(float3x3 m)
{
    float d = m[0].x * (m[1].y * m[2].z - m[2].y * m[1].z) -
              m[0].y * (m[1].x * m[2].z - m[1].z * m[2].x) +
              m[0].z * (m[1].x * m[2].y - m[1].y * m[2].x);
              
    float id = 1.0f / d;
    
    float3x3 c = float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    
    c[0].x = id * (m[1].y * m[2].z - m[2].y * m[1].z);
    c[0].y = id * (m[0].z * m[2].y - m[0].y * m[2].z);
    c[0].z = id * (m[0].y * m[1].z - m[0].z * m[1].y);
    c[1].x = id * (m[1].z * m[2].x - m[1].x * m[2].z);
    c[1].y = id * (m[0].x * m[2].z - m[0].z * m[2].x);
    c[1].z = id * (m[1].x * m[0].z - m[0].x * m[1].z);
    c[2].x = id * (m[1].x * m[2].y - m[2].x * m[1].y);
    c[2].y = id * (m[2].x * m[0].y - m[0].x * m[2].y);
    c[2].z = id * (m[0].x * m[1].y - m[1].x * m[0].y);
    
    return c;
}

float3 xyYToXYZ(float3 xyY)
{
    if (xyY.y == 0.0f)
    {
        return float3(0, 0, 0);
    }

    float Y = xyY.z;
    float X = (xyY.x * Y) / xyY.y;
    float Z = ((1.0f - xyY.x - xyY.y) * Y) / xyY.y;

    return float3(X, Y, Z);
}

float3 Unproject(float2 xy)
{
    return xyYToXYZ(float3(xy.x, xy.y, 1));
}

float3x3 PrimariesToMatrix(float2 xy_red, float2 xy_green, float2 xy_blue, float2 xy_white)
{
    float3 XYZ_red = Unproject(xy_red);
    float3 XYZ_green = Unproject(xy_green);
    float3 XYZ_blue = Unproject(xy_blue);

    float3 XYZ_white = Unproject(xy_white);

    float3x3 temp = float3x3(XYZ_red.x, XYZ_green.x, XYZ_blue.x,
                     1.0f, 1.0f, 1.0f,
                     XYZ_red.z, XYZ_green.z, XYZ_blue.z);

    float3x3 inverse = InverseMat(temp);
    float3 scale = mul(XYZ_white, inverse);

    return float3x3(scale.x * XYZ_red.x, scale.y * XYZ_green.x, scale.z * XYZ_blue.x,
                scale.x * XYZ_red.y, scale.y * XYZ_green.y, scale.z * XYZ_blue.y,
                scale.x * XYZ_red.z, scale.y * XYZ_green.z, scale.z * XYZ_blue.z);
}

float3x3 ComputeCompressionMatrix(float2 xyR, float2 xyG, float2 xyB, float2 xyW, float3 compression)
{
    float2 R = ((xyR - xyW) * (1.0f / (1.0f - compression.x))) + xyW;
    float2 G = ((xyG - xyW) * (1.0f / (1.0f - compression.y))) + xyW;
    float2 B = ((xyB - xyW) * (1.0f / (1.0f - compression.z))) + xyW;
    float2 W = xyW;

    return PrimariesToMatrix(R, G, B, W);
}

float3 OpenDomainToNormalizedLog2(float3 openDomain, float minimum_ev, float maximum_ev)
{
    float total_exposure = maximum_ev - minimum_ev;

    float3 output_log = clamp(log2(openDomain / AgXMiddleGray), minimum_ev, maximum_ev);

    return (output_log - minimum_ev) / total_exposure;
}

float AgXScale(float x_pivot, float y_pivot, float slope_pivot, float power)
{
    return pow(pow((slope_pivot * x_pivot), -power) * (pow((slope_pivot * (x_pivot / y_pivot)), power) - 1.0), -1.0 / power);
}

float AgXHyperbolic(float x, float power)
{
    return x / pow(1.0 + pow(x, power), 1.0f / power);
}

float AgXTerm(float x, float x_pivot, float slope_pivot, float scale)
{
    return (slope_pivot * (x - x_pivot)) / scale;
}

float AgXCurve(float x, float x_pivot, float y_pivot, float slope_pivot, float toe_power, float shoulder_power, float scale)
{
    if (scale < 0.0f)
    {
        return scale * AgXHyperbolic(AgXTerm(x, x_pivot, slope_pivot, scale), toe_power) + y_pivot;
    }
    else
    {
        return scale * AgXHyperbolic(AgXTerm(x, x_pivot, slope_pivot, scale), shoulder_power) + y_pivot;
    }
}

float AgXFullCurve(float x, float x_pivot, float y_pivot, float slope_pivot, float toe_power, float shoulder_power)
{
    float scale_x_pivot = x >= x_pivot ? 1.0f - x_pivot : x_pivot;
    float scale_y_pivot = x >= x_pivot ? 1.0f - y_pivot : y_pivot;

    float toe_scale = AgXScale(scale_x_pivot, scale_y_pivot, slope_pivot, toe_power);
    float shoulder_scale = AgXScale(scale_x_pivot, scale_y_pivot, slope_pivot, shoulder_power);

    float scale = x >= x_pivot ? shoulder_scale : -toe_scale;

    return AgXCurve(x, x_pivot, y_pivot, slope_pivot, toe_power, shoulder_power, scale);
}

float3 AgX(float3 color)
{
    float3 workingColor = max(color, 0.0f) * pow(2.0f, Exposure);
    
    float3x3 sRGB_to_XYZ = PrimariesToMatrix(float2(0.64, 0.33),
                                         float2(0.3, 0.6),
                                         float2(0.15, 0.06),
                                         float2(0.3127, 0.3290));

    float3x3 adjusted_to_XYZ = ComputeCompressionMatrix(float2(0.64, 0.33),
                                                    float2(0.3, 0.6),
                                                    float2(0.15, 0.06),
                                                    float2(0.3127, 0.3290), float3(AgXCompressionR, AgXCompressionG, AgXCompressionB));
    								
    float3x3 XYZ_to_adjusted = InverseMat(adjusted_to_XYZ);

    float3 xyz = mul(workingColor, sRGB_to_XYZ);
    float3 adjustedRGB = mul(xyz, XYZ_to_adjusted);

    float x_pivot = abs(MIN_EV) / (MAX_EV - MIN_EV);
    float y_pivot = 0.5f;

    float3 logV = OpenDomainToNormalizedLog2(adjustedRGB, MIN_EV, MAX_EV);

    float outputR = AgXFullCurve(logV.r, x_pivot, y_pivot, AgXSlope, AgXToePower, AgXShoulderPower);
    float outputG = AgXFullCurve(logV.g, x_pivot, y_pivot, AgXSlope, AgXToePower, AgXShoulderPower);
    float outputB = AgXFullCurve(logV.b, x_pivot, y_pivot, AgXSlope, AgXToePower, AgXShoulderPower);

    workingColor = clamp(float3(outputR, outputG, outputB), 0.0, 1.0);

    float3 luminanceWeight = float3(0.2126729f, 0.7151522f, 0.0721750f);
    float3 desaturation = float3(dot(workingColor, luminanceWeight).xxx);
    workingColor = lerp(desaturation, workingColor, AgXSaturation);
    workingColor = clamp(workingColor, 0.f, 1.f);

    // Lerp between raw and image
    return lerp(color, workingColor, AgXLerp);
}

PixelOutput main(VertexToPixel input)
{
    PixelOutput returnValue;

    returnValue.Color.rgba = 1.0f;
    const float3 resource = fullscreenTexture1.Sample(defaultSampler, input.UV.xy).rgb;
	
	// No Tonemapping
	//{
    //     returnValue.myColor.rgb = resource;
    //}
	// No Tonemapping

	// Reinhard
    if (IsReinhard)
    {
        returnValue.Color.rgb = resource / (resource + 1.0f);
    }
	// Reinhard
    
	// Uncharted 2
    if (IsUncharted)
    {
	    const float3 whitePoint = (float3)WhitePointColor * WhitePointIntensity;
	    const float exposure = Exposure;
        returnValue.Color.rgb = Uncharted2Tonemap(resource * exposure) / Uncharted2Tonemap(whitePoint);
    }
	// Uncharted 2
    
    // ACES
    if (IsACES)
    {
        const float3 whitePoint = (float3) WhitePointColor * WhitePointIntensity;
        const float exposure = Exposure;
        returnValue.Color.rgb = ACESFitted(resource * exposure) / ACESFitted(whitePoint);
        //returnValue.Color.rgb = ACESApprox(resource * exposure) / ACESApprox(whitePoint);
    }
    // ACES
    
    // AgX
    if (IsAgX)
    {
        returnValue.Color.rgb = AgX(resource);
    }
    // AgX
    
    returnValue.Color.a = 0.0f;
    return returnValue;
};