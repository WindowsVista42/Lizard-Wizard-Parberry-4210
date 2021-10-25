#include "TonemapEffect_Common.hlsli"
#include "PostProcess_Common.hlsli"

//=================================================================================================
//
//  Baking Lab
//  by MJP and David Neubelt
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//=================================================================================================

// The code in this file was originally written by Stephen Hill (@self_shadow), who deserves all
// credit for coming up with this fit and implementing it. Buy him a beer next time you see him. :)

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static const float3x3 ACESInputMat =
{
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367},
    {-0.10208,  1.10813, -0.00605},
    {-0.00327, -0.07276,  1.07602}
};

float3 RRTAndODTFit(float3 v)
{
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
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

float Luminance(float3 color) {
    return dot(color, float3(0.299f, 0.587f, 0.114f));
}

float3 BoxBlur3x3(uint2 dim, float2 uv, float intensity) {
    float2 d = (float2(1.0f, 1.0f) / dim) * intensity;
    const float kernel = 1.0f / 9.0f;

    float3 output;
    output  = kernel * Color.Sample(Sampler, float2(uv.x - d.x, uv.y - d.y)).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x      , uv.y - d.y)).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x + d.x, uv.y - d.y)).xyz;

    output += kernel * Color.Sample(Sampler, float2(uv.x - d.x, uv.y      )).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x      , uv.y      )).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x + d.x, uv.y      )).xyz;

    output += kernel * Color.Sample(Sampler, float2(uv.x - d.x, uv.y + d.y)).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x      , uv.y + d.y)).xyz;
    output += kernel * Color.Sample(Sampler, float2(uv.x + d.x, uv.y + d.y)).xyz;

    return output;
}

[RootSignature(TonemapEffectRS)]
PixelOutput main(VertexOutput input) {
    PixelOutput output;

    const float3 Tint = float3(TintR, TintG, TintB);

    const float3 blurred_color = BoxBlur3x3(uint2(DimensionsX, DimensionsY), input.Texture, Glasses);
    const float3 tinted_color = blurred_color * Tint;
    const float lum = Luminance(tinted_color);
    const float3 saturated_color = lerp(float3(lum, lum, lum), tinted_color, Saturation);
    const float3 tonemap_color = ACESFitted(saturated_color);

    output.Color = float4(tonemap_color, 1.0f);

    return output;
}